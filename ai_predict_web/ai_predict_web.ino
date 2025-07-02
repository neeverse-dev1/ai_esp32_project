#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <NewPing.h>
#include <ArduinoJson.h> // JSON 파싱/생성을 위해 필요
#include <FS.h>          // 파일 시스템 (SPIFFS) 사용을 위해 필요
#include <SPIFFS.h>      // SPIFFS 사용을 위해 필요

#include "config.h"    // Wi-Fi 설정 및 임계값 정의 (확인 필요)
#include "web_page.h"  // 웹 페이지 HTML/JS 내용

// --- DHT11 센서 설정 ---
#define DHTPIN 4       // DHT11 센서 데이터 핀 (ESP32 GPIO 4)
#define DHTTYPE DHT11  // DHT 11 타입
DHT dht(DHTPIN, DHTTYPE);

// --- HC-SR04 센서 설정 ---
#define TRIGGER_PIN 5  // 초음파 센서 Trig 핀 (ESP32 GPIO 5)
#define ECHO_PIN 18    // 초음파 센서 Echo 핀 (ESP32 GPIO 18)
#define MAX_DISTANCE 200 // 최대 거리 (cm). 200cm까지 측정
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

// --- 웹 서버 설정 ---
WebServer server(80); // 웹 서버를 80번 포트로 초기화

// --- 예측값을 저장할 전역 변수 ---
volatile float predictedTemperature = 0.0; 
volatile float predictedHumidity = 0.0;
volatile int predictedDistance = 0;

// --- 전역 변수 정의 (config.h에서 extern으로 선언된 변수들을 여기서 실제 메모리에 할당) ---
// 임계값 변수들
float HIGH_TEMP_THRESHOLD = 28.0;     // 고온 경고 임계값 (섭씨)
float HIGH_HUMIDITY_THRESHOLD = 70.0; // 고습 경고 임계값 (%)
int OBJECT_CLOSE_THRESHOLD = 30;      // 물체 근접 경고 임계값 (cm)
int OBJECT_VERY_CLOSE_THRESHOLD = 10; // 물체 매우 근접 경고 임계값 (cm)

// 센서 값 및 상태 변수 (volatile 추가 - loop와 웹 서버 핸들러 간 공유)
volatile float currentTemp = 0.0;
volatile float currentHum = 0.0;
volatile int currentDistance = 0;
String currentStatus = "정상"; // 현재 상태 메시지 (웹으로만 전송, 시리얼 플로터에 영향 없음)

// --- 임계값 저장/로드 파일 경로 ---
const char* THRESHOLDS_FILE = "/thresholds.json";

// --- 함수 정의 ---

// 임계값을 SPIFFS에 저장하는 함수
void saveThresholds() {
  File file = SPIFFS.open(THRESHOLDS_FILE, "w");
  if (!file) {
    Serial.println("파일 열기 실패: " + String(THRESHOLDS_FILE));
    return;
  }

  StaticJsonDocument<256> doc;
  doc["temp"] = HIGH_TEMP_THRESHOLD;
  doc["hum"] = HIGH_HUMIDITY_THRESHOLD;
  doc["close"] = OBJECT_CLOSE_THRESHOLD;
  doc["veryClose"] = OBJECT_VERY_CLOSE_THRESHOLD;

  if (serializeJson(doc, file) == 0) {
    Serial.println("임계값 파일 쓰기 실패");
  } else {
    Serial.println("임계값 저장 완료");
  }
  file.close();
}

// SPIFFS에서 임계값을 로드하는 함수
void loadThresholds() {
  if (!SPIFFS.exists(THRESHOLDS_FILE)) {
    Serial.println("임계값 파일이 존재하지 않습니다. 기본값 사용.");
    saveThresholds(); // 기본값으로 파일 생성 및 저장
    return;
  }

  File file = SPIFFS.open(THRESHOLDS_FILE, "r");
  if (!file) {
    Serial.println("파일 열기 실패: " + String(THRESHOLDS_FILE));
    return;
  }

  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    Serial.print("임계값 파일 파싱 실패: ");
    Serial.println(error.c_str());
    file.close();
    return;
  }

  HIGH_TEMP_THRESHOLD = doc["temp"] | 28.0; // 파일에 값이 없으면 기본값 사용
  HIGH_HUMIDITY_THRESHOLD = doc["hum"] | 70.0;
  OBJECT_CLOSE_THRESHOLD = doc["close"] | 30;
  OBJECT_VERY_CLOSE_THRESHOLD = doc["veryClose"] | 10;

  Serial.println("임계값 로드 완료:");
  Serial.printf("  온도: %.1f, 습도: %.1f, 근접: %d, 매우근접: %d\n", 
               HIGH_TEMP_THRESHOLD, HIGH_HUMIDITY_THRESHOLD, 
               OBJECT_CLOSE_THRESHOLD, OBJECT_VERY_CLOSE_THRESHOLD);
  file.close();
}

// --- 센서 값 읽고 상태 업데이트 함수 (시리얼 플로터 출력 포함) ---
void readAndAnalyzeSensors() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("DHT11 센서 읽기 실패!");
    // 오류가 발생해도 이전 값을 유지하거나 0 등으로 설정할 수 있습니다.
    // currentHum = 0.0; currentTemp = 0.0; // 필요하다면 초기화
  } else {
    currentHum = h;
    currentTemp = t;
  }

  unsigned int duration = sonar.ping_cm();
  
  // NewPing 라이브러리는 0을 반환할 경우 '핑 아웃 오브 레인지' 또는 오류를 의미
  if (duration == 0) {
    currentDistance = MAX_DISTANCE + 1; // 측정 불가 또는 최대 거리 초과
  } else {
    currentDistance = duration;
  }

  // 상태 메시지 업데이트 (currentStatus 변수)
  String newStatus = "정상";
  
  if (currentTemp > HIGH_TEMP_THRESHOLD) {
    newStatus = "경고: 고온 감지!";
  }
  if (currentHum > HIGH_HUMIDITY_THRESHOLD) {
    if (newStatus != "정상") newStatus += ", 고습";
    else newStatus = "경고: 고습 감지!";
  }

  if (currentDistance <= OBJECT_VERY_CLOSE_THRESHOLD) {
    newStatus = "위험: 물체 매우 근접!";
  } else if (currentDistance <= OBJECT_CLOSE_THRESHOLD) {
    if (newStatus == "정상") newStatus = "감지: 물체 근접!";
    else newStatus += ", 물체 근접";
  }

  // 복합 조건 처리 (가장 높은 우선순위)
  if (currentTemp > HIGH_TEMP_THRESHOLD && currentHum > HIGH_HUMIDITY_THRESHOLD && currentDistance <= OBJECT_CLOSE_THRESHOLD) {
    newStatus = "위험: 고온/고습 및 물체 근접!";
  }
  
  currentStatus = newStatus;

  // 시리얼 플로터 출력을 위해 숫자 값만 탭으로 구분하여 출력
  Serial.print(currentTemp); // 온도
  Serial.print("\t");
  Serial.print(currentHum); // 습도
  Serial.print("\t");
  Serial.println(currentDistance); // 거리 (줄바꿈)
}

// --- 웹 서버 핸들러 ---
void handleRoot() {
  server.send_P(200, "text/html;charset=UTF-8", MAIN_page);
}

// 센서 데이터 및 예측값을 JSON으로 반환하는 핸들러
void handleData() {
  readAndAnalyzeSensors(); // 최신 센서 값 및 상태 업데이트

  StaticJsonDocument<256> doc; // 예측값을 위해 약간 더 크게 설정
  doc["temperature"] = currentTemp;
  doc["humidity"] = currentHum;
  doc["distance"] = currentDistance;
  doc["status"] = currentStatus; // 현재 센서값에 기반한 상태

  // 예측값 추가!
  doc["predictedTemperature"] = predictedTemperature;
  doc["predictedHumidity"] = predictedHumidity;
  doc["predictedDistance"] = predictedDistance;

  String jsonOutput;
  serializeJson(doc, jsonOutput);

  server.send(200, "application/json;charset=UTF-8", jsonOutput);
}

// --- 임계값 가져오기 핸들러 ---
void handleGetThresholds() {
  StaticJsonDocument<200> doc;
  doc["temp"] = HIGH_TEMP_THRESHOLD;
  doc["hum"] = HIGH_HUMIDITY_THRESHOLD;
  doc["close"] = OBJECT_CLOSE_THRESHOLD;
  doc["veryClose"] = OBJECT_VERY_CLOSE_THRESHOLD;

  String jsonOutput;
  serializeJson(doc, jsonOutput);

  server.send(200, "application/json;charset=UTF-8", jsonOutput);
}

// --- 임계값 설정 핸들러 (JSON 본문 처리) ---
void handleSetThresholds() {
  if (server.hasArg("plain")) { // "plain" 인자(POST 요청 본문)가 있는지 확인
    String payload = server.arg("plain"); // POST 요청의 본문 전체를 String으로 가져옴

    Serial.print("수신된 JSON: ");
    Serial.println(payload);

    StaticJsonDocument<256> doc; // 적절한 크기
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      Serial.print("JSON 파싱 실패: ");
      Serial.println(error.c_str());
      server.send(400, "text/plain", "Invalid JSON");
      return;
    }

    if (doc.containsKey("temp")) {
      HIGH_TEMP_THRESHOLD = doc["temp"].as<float>();
    }
    if (doc.containsKey("hum")) {
      HIGH_HUMIDITY_THRESHOLD = doc["hum"].as<float>();
    }
    if (doc.containsKey("close")) {
      OBJECT_CLOSE_THRESHOLD = doc["close"].as<int>();
    }
    if (doc.containsKey("veryClose")) {
      OBJECT_VERY_CLOSE_THRESHOLD = doc["veryClose"].as<int>();
    }
    
    saveThresholds(); // 변경된 임계값 SPIFFS에 저장
    server.send(200, "text/plain", "Thresholds updated successfully");
    return;
  }
  server.send(400, "text/plain", "Bad Request: No payload"); // 내용이 없거나 잘못된 요청
}

// --- setup() 함수 ---
void setup() {
  Serial.begin(115200);

  // SPIFFS 초기화
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS 마운트 실패");
    return;
  }
  Serial.println("SPIFFS 마운트 성공");
  loadThresholds(); // 저장된 임계값 로드

  dht.begin(); // DHT 센서 시작

  Serial.print("Wi-Fi 연결 중: ");
  Serial.println(ssid); // config.h에서 정의된 ssid 사용
  WiFi.begin(ssid, password); // config.h에서 정의된 password 사용

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(1000);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi 연결 성공!");
    Serial.print("ESP32 IP 주소: ");
    Serial.println(WiFi.localIP());

    // --- 맥주소 출력 추가 ---
    Serial.print("ESP32 MAC 주소: ");
    Serial.println(WiFi.macAddress());
    // ----------------------

    // 예측값을 받을 POST 핸들러
    server.on("/predict", HTTP_POST, []() {
      if (server.hasArg("plain")) {
        String body = server.arg("plain");
        Serial.println("Received prediction: " + body); // 디버그용
        
        DynamicJsonDocument doc(256); // 적절한 메모리 크기
        DeserializationError error = deserializeJson(doc, body);

        if (error) {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
          server.send(400, "text/plain", "Bad Request");
          return;
        }

        predictedTemperature = doc["temp_pred"].as<float>();
        predictedHumidity = doc["hum_pred"].as<float>();
        predictedDistance = doc["dist_pred"].as<int>();
        
        Serial.print("Predicted T: "); Serial.println(predictedTemperature);
        Serial.print("Predicted H: "); Serial.println(predictedHumidity);
        Serial.print("Predicted D: "); Serial.println(predictedDistance);

        server.send(200, "text/plain", "Prediction received");
      } else {
        server.send(400, "text/plain", "Bad Request");
      }
    });

    // --- 웹 서버 핸들러 매핑 ---
    server.on("/", handleRoot);
    server.on("/data", handleData); // handleData 함수를 사용하도록 수정
    server.on("/getThresholds", handleGetThresholds); 
    server.on("/setThresholds", HTTP_POST, handleSetThresholds);

    server.begin(); // 웹 서버 시작
    Serial.println("HTTP 웹 서버 시작됨.");
  } else {
    Serial.println("\nWi-Fi 연결 실패! 다시 시도하거나 SSID/PW 확인하세요.");
  }
  Serial.println("----------------------------");
}

// --- loop() 함수 ---
void loop() {
  // 1. 웹 서버 클라이언트 요청을 처리합니다.
  // 이 함수가 Python 스크립트로부터의 HTTP POST 요청 (예측값)을 받아서 처리합니다.
  server.handleClient(); 

  // 2. 센서 값을 읽고, 웹 페이지에 표시될 currentTemp, currentHum, currentDistance, currentStatus를 업데이트합니다.
  // 이 함수 내부에서 센서 데이터의 시리얼 플로터 출력이 이루어지고 있습니다.
  readAndAnalyzeSensors(); 
  
  // 3. (IMPORTANT) 시리얼 포트로부터 데이터를 읽고 파싱하는 모든 로직을 제거해야 합니다.
  // 이 부분은 Python이 HTTP로 통신하고, ESP32는 자체 센서 값을 시리얼로 출력하는 경우 불필요합니다.
  // 즉, 아래와 같은 코드가 있다면 삭제해주세요:
  //if (Serial.available()) {
  //   String line = Serial.readStringUntil('\n');
  //   line.trim();
  //  // ... (여기서 line을 파싱하는 모든 코드) ...
  // }

  // 4. 센서 값을 읽는 간격을 조절합니다. (예: 1초 대기)
  delay(1000); 
}