#include <DHT.h> // DHT 센서 라이브러리
#include <DHT_U.h> // DHT 유틸리티 라이브러리 (필요할 수 있음)

// DHT11 센서 핀 설정
#define DHTPIN 4        // DHT11 데이터 핀 (ESP32의 GPIO 4번에 연결)
#define DHTTYPE DHT11   // DHT 센서 타입 (DHT11)

// HC-SR04 센서 핀 설정
#define ECHO_PIN 18     // HC-SR04 에코 핀 (ESP32의 GPIO 16번에 연결)
#define TRIG_PIN 5     // HC-SR04 트리거 핀 (ESP32의 GPIO 17번에 연결)

DHT dht(DHTPIN, DHTTYPE); // DHT 객체 생성

void setup() {
  Serial.begin(115200);   // 시리얼 통신 시작 (보드레이트 115200)
  dht.begin();            // DHT 센서 시작

  // HC-SR04 핀 설정
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  delay(100); // 센서 초기화 대기
  Serial.println("ESP32 DHT11 and HC-SR04 Sensor Test");
  Serial.println("------------------------------------");
}

void loop() {
  // --- DHT11 센서 데이터 읽기 ---
  float humidity = dht.readHumidity();    // 습도 읽기
  float temperature = dht.readTemperature(); // 온도 읽기

  // 데이터 읽기 실패 확인
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    Serial.print("Humidity:");
    Serial.print(humidity);
    Serial.print(",");
    //Serial.print(" %\t");
    Serial.print("Temperature:");
    Serial.print(temperature);
    Serial.print(",");
    //Serial.println(" *C");
  }

  // --- HC-SR04 센서 데이터 읽기 ---
  digitalWrite(TRIG_PIN, LOW); // 트리거 핀 LOW로 초기화
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); // 트리거 핀 HIGH로 10us 유지 (초음파 발생)
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH); // 에코 핀에서 초음파 반사 시간 측정 (HIGH 상태 유지 시간)

  // 거리를 계산 (소리의 속도 340m/s = 0.034cm/us, 왕복 거리이므로 2로 나눔)
  float distanceCm = duration * 0.034 / 2;

  if (distanceCm == 0) { // 비정상적인 거리값 (측정 불가)
    Serial.println("Distance: Out of range or sensor error!");
  } else {
    Serial.print("Distance:");
    Serial.print(distanceCm);
    Serial.print(",");
    
    //Serial.println(" cm");
  }
  Serial.println();
  // Serial.println("---");
  
  delay(2000); // 2초마다 센서 값 읽기
}

