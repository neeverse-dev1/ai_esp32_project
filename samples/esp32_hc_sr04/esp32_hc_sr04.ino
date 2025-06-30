// ESP32 HC-SR04 초음파 센서 샘플 코드

// Trig 핀과 Echo 핀이 연결된 ESP32 GPIO 핀 번호 정의
const int trigPin = 5; // Trig 핀을 ESP32 GPIO 5에 연결
const int echoPin = 18; // Echo 핀을 ESP32 GPIO 18에 연결

// 거리 측정을 위한 변수
long duration; // 초음파가 돌아오는 데 걸린 시간
int distanceCm; // 센서에서 측정된 거리 (cm)
int distanceInch; // 센서에서 측정된 거리 (인치)

void setup() {
  // 시리얼 통신 초기화 (결과를 PC에서 확인하기 위함)
  Serial.begin(115200);

  // Trig 핀은 출력으로 설정 (초음파 발사)
  pinMode(trigPin, OUTPUT);
  // Echo 핀은 입력으로 설정 (초음파 수신)
  pinMode(echoPin, INPUT);
}

void loop() {
  // Trig 핀을 LOW로 2 마이크로초 동안 유지하여 깨끗한 펄스 생성 준비
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Trig 핀을 HIGH로 10 마이크로초 동안 유지하여 초음파 발사
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW); // 초음파 발사 완료

  // Echo 핀의 HIGH 상태 지속 시간을 측정 (초음파가 반사되어 돌아오는 시간)
  duration = pulseIn(echoPin, HIGH);

  // 시간(마이크로초)을 거리(센티미터)로 변환
  // 소리의 속도는 약 343m/s 또는 0.0343cm/microsecond
  // 거리는 (시간 * 소리의 속도) / 2 (왕복 거리이므로 2로 나눔)
  distanceCm = duration * 0.0343 / 2;
  
  // 센티미터를 인치로 변환 (1인치 = 2.54cm)
  distanceInch = distanceCm / 2.54;

  // 시리얼 모니터에 결과 출력
  Serial.print("Distance: ");
  Serial.print(distanceCm);
  Serial.print(" cm");
  Serial.print(" | ");
  Serial.print(distanceInch);
  Serial.println(" inch");

  // 다음 측정을 위해 잠시 대기
  delay(1000); // 1초마다 측정
}
