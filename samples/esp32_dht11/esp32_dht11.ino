#include "DHT.h" // DHT 센서 라이브러리 포함
#include "Adafruit_Sensor.h" // Adafruit 통합 센서 라이브러리 포함

// DHT11 센서가 연결된 ESP32 GPIO 핀 번호 정의
#define DHTPIN 4 

// DHT 센서의 타입 정의 (DHT11, DHT22, DHT21 중 선택)
#define DHTTYPE DHT11 // DHT 11

// DHT 객체 생성
// DHT dht(핀 번호, 센서 타입)
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200); // 시리얼 통신 시작 (보통 115200 bps 사용)
  Serial.println(F("DHT11 테스트!")); // 시작 메시지 출력

  dht.begin(); // DHT 센서 초기화
}

void loop() {
  // 측정 간 간격 (DHT11은 약 250ms가 필요하며, 2초 정도 권장)
  delay(2000); 

  // 습도 읽기
  float h = dht.readHumidity();
  // 온도 읽기 (섭씨)
  float t = dht.readTemperature();
  // 온도 읽기 (화씨) - 필요하다면 사용
  // float f = dht.readTemperature(true);

  // 센서 읽기 실패 여부 확인
  if (isnan(h) || isnan(t)) {
    Serial.println(F("DHT 센서에서 데이터를 읽는 데 실패했습니다!"));
    return;
  }

  // 시리얼 모니터에 값 출력
  Serial.print(F("습도: "));
  Serial.print(h);
  Serial.print(F("%  온도: "));
  Serial.print(t);
  Serial.println(F("°C"));

  // 필요하다면 화씨도 출력
  // Serial.print(F("  온도: "));
  // Serial.print(f);
  // Serial.println(F("°F"));
}
