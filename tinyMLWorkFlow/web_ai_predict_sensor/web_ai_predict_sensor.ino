#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <NewPing.h>
#include <ArduinoJson.h> // JSON 파싱/생성을 위해 필요
#include <FS.h>          // 파일 시스템 (SPIFFS) 사용을 위해 필요
#include <SPIFFS.h>      // SPIFFS 사용을 위해 필요

#include "config.h"   // Wi-Fi 설정 및 임계값 정의
#include "web_page.h" // 웹 페이지 HTML/JS 내용


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

// --- 전역 변수 정의 (config.h에서 extern으로 선언된 변수들을 여기서 실제 메모리에 할당) ---
float HIGH_TEMP_THRESHOLD = 28.0;     // 고온 경고 임계값 (섭씨)
float HIGH_HUMIDITY_THRESHOLD = 70.0; // 고습 경고 임계값 (%)
int OBJECT_CLOSE_THRESHOLD = 30;      // 물체 근접 경고 임계값 (cm)
int OBJECT_VERY_CLOSE_THRESHOLD = 10; // 물체 매우 근접 경고 임계값 (cm)

// 센서 값 및 상태 변수
float currentTemp = 0.0;
float currentHum = 0.0;
int currentDistance = 0;
String currentStatus = "정상"; // 현재 상태 메시지

// --- 임계값 저장/로드 파일 경로 ---
const char* THRESHOLDS_FILE = "/thresholds.json";

// --- 함수 정의 ---
// (모든 함수 정의는 setup()이나 loop() 밖에 있어야 합니다!)

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

// ... (이전 C++ 코드 생략) ...

// --- 센서 값 읽고 상태 업데이트 함수 ---
void readAndAnalyzeSensors() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("DHT11 센서 읽기 실패!");
  } else {
    currentHum = h;
    currentTemp = t;
  }

  unsigned int duration = sonar.ping_cm();
  
  if (duration == 0) {
    currentDistance = MAX_DISTANCE + 1;
  } else {
    currentDistance = duration;
  }

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

  if (currentTemp > HIGH_TEMP_THRESHOLD && currentHum > HIGH_HUMIDITY_THRESHOLD && currentDistance <= OBJECT_CLOSE_THRESHOLD) {
    newStatus = "위험: 고온/고습 및 물체 근접!";
  }
  
  currentStatus = newStatus;
  //Serial.println("현재 상태: " + currentStatus);
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

void handleData() {
  readAndAnalyzeSensors(); // 최신 센서 값 및 상태 업데이트

  StaticJsonDocument<200> doc;
  doc["temperature"] = currentTemp;
  doc["humidity"] = currentHum;
  doc["distance"] = currentDistance;
  doc["status"] = currentStatus;

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

  dht.begin();

  Serial.print("Wi-Fi 연결 중: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

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

    server.on("/", handleRoot);
    server.on("/data", handleData);
    server.on("/getThresholds", handleGetThresholds); 
    server.on("/setThresholds", HTTP_POST, handleSetThresholds);

    server.begin();
    Serial.println("HTTP 웹 서버 시작됨.");
  } else {
    Serial.println("\nWi-Fi 연결 실패! 다시 시도하거나 SSID/PW 확인하세요.");
  }
  Serial.println("----------------------------");
}

// --- loop() 함수 ---
void loop() {
  server.handleClient();
}
