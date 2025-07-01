#include <DHT.h>       // DHT11 온습도 센서 라이브러리
#include <NewPing.h>   // HC-SR04 초음파 센서 라이브러리

// --- DHT11 센서 설정 ---
#define DHTPIN 4       // DHT11 센서 데이터 핀 (ESP32 GPIO 4)
#define DHTTYPE DHT11  // DHT 11 타입
DHT dht(DHTPIN, DHTTYPE);

// --- HC-SR04 센서 설정 ---
#define TRIGGER_PIN 5  // 초음파 센서 Trig 핀 (ESP32 GPIO 5)
#define ECHO_PIN 18    // 초음파 센서 Echo 핀 (ESP32 GPIO 18)
#define MAX_DISTANCE 200 // 최대 거리 (cm). 200cm까지 측정
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

// --- 분석을 위한 임계값 설정 ---
const float HIGH_TEMP_THRESHOLD = 28.0; // 고온 경고 임계값 (섭씨)
const float HIGH_HUMIDITY_THRESHOLD = 70.0; // 고습 경고 임계값 (%)
const int OBJECT_CLOSE_THRESHOLD = 30; // 물체 근접 경고 임계값 (cm)
const int OBJECT_VERY_CLOSE_THRESHOLD = 10; // 물체 매우 근접 경고 임계값 (cm)

void setup() {
  Serial.begin(115200); // 시리얼 통신 시작
  dht.begin();          // DHT 센서 시작
  Serial.println("ESP32 센서 AI 분석 시작...");
  Serial.println("----------------------------");
}

void loop() {
  delay(2000); // 2초마다 센서 값 읽기 (센서 안정화 및 루프 지연)

  // --- 1. DHT11 온습도 값 읽기 ---
  float h = dht.readHumidity();    // 습도 읽기
  float t = dht.readTemperature(); // 온도 읽기 (섭씨)

  // 센서 읽기 실패 확인
  if (isnan(h) || isnan(t)) {
    Serial.println("DHT11 센서 읽기 실패!");
  } else {
    Serial.print("온도: ");
    Serial.print(t);
    Serial.print(" *C, 습도: ");
    Serial.print(h);
    Serial.println(" %");

    // --- DHT11 규칙 기반 분석 ---
    if (t > HIGH_TEMP_THRESHOLD) {
      Serial.print("  [경고] 고온 감지! (");
      Serial.print(t);
      Serial.println("*C)");
    }
    if (h > HIGH_HUMIDITY_THRESHOLD) {
      Serial.print("  [경고] 고습 감지! (");
      Serial.print(h);
      Serial.println("%)");
    }
  }

  // --- 2. HC-SR04 거리 값 읽기 ---
  // Ping 결과는 마이크로초(us) 단위, cm로 변환
  unsigned int duration = sonar.ping();
  int distanceCm = sonar.convert_cm(duration);

  if (distanceCm == 0) { // 0은 측정이 불가능했음을 의미 (너무 멀거나, 감지 안되거나)
    Serial.println("거리: 측정 불가 또는 200cm 이상");
  } else {
    Serial.print("거리: ");
    Serial.print(distanceCm);
    Serial.println(" cm");

    // --- HC-SR04 규칙 기반 분석 ---
    if (distanceCm < OBJECT_VERY_CLOSE_THRESHOLD) {
      Serial.print("  [위험] 물체 매우 근접!! (");
      Serial.print(distanceCm);
      Serial.println("cm)");
    } else if (distanceCm < OBJECT_CLOSE_THRESHOLD) {
      Serial.print("  [감지] 물체 근접! (");
      Serial.print(distanceCm);
      Serial.println("cm)");
    }
  }

  // --- 3. 복합 분석 ---
  // DHT11과 HC-SR04 데이터가 모두 유효할 때만 복합 분석 수행
  if (!isnan(h) && !isnan(t) && distanceCm != 0) {
    if (t > HIGH_TEMP_THRESHOLD && h > HIGH_HUMIDITY_THRESHOLD && distanceCm < OBJECT_CLOSE_THRESHOLD) {
      Serial.println("  [종합 분석] 고온/고습 환경에서 물체 근접!");
      // 여기에 추가적인 액션 (예: 알림, 릴레이 제어 등)을 넣을 수 있습니다.
    }
  }

  Serial.println("----------------------------");
}
