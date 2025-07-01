#include <TensorFlowLite.h>
#include "model.h"
#include "DHT.h"

// 센서 핀 정의
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define TRIG_PIN 5
#define ECHO_PIN 18

// TensorFlow 설정
#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
// #include "tensorflow/lite/version.h"

// 입력/출력 버퍼 설정
constexpr int kTensorArenaSize = 2 * 1024;
uint8_t tensor_arena[kTensorArenaSize];

// 모델 포인터
const tflite::Model* model;
tflite::MicroInterpreter* interpreter;
TfLiteTensor* input;
TfLiteTensor* output;

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // TinyML 모델 불러오기
  model = tflite::GetModel(model_data);
  static tflite::MicroMutableOpResolver<4> resolver;
  resolver.AddFullyConnected();
  resolver.AddRelu();
  resolver.AddSoftmax();
  resolver.AddReshape();

  static tflite::MicroInterpreter static_interpreter(model, resolver, tensor_arena, kTensorArenaSize);
  interpreter = &static_interpreter;
  interpreter->AllocateTensors();

  input = interpreter->input(0);
  output = interpreter->output(0);
}

void loop() {
  // 센서값 측정
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = duration * 0.034 / 2.0;

  // 입력값 설정 (정규화 필요 시 직접 적용)
  input->data.f[0] = temp;
  input->data.f[1] = hum;
  input->data.f[2] = distance;

  // 추론
  TfLiteStatus invoke_status = interpreter->Invoke();
  if (invoke_status != kTfLiteOk) {
    Serial.println("Invoke failed!");
    return;
  }

  // 출력값 확인
  float pred_0 = output->data.f[0]; // 클래스 0의 확률
  float pred_1 = output->data.f[1]; // 클래스 1의 확률

  int predicted_class = (pred_1 > pred_0) ? 1 : 0;

  // 결과 출력
  Serial.print("Temp: "); Serial.print(temp);
  Serial.print(" Hum: "); Serial.print(hum);
  Serial.print(" Dist: "); Serial.print(distance);
  Serial.print(" → 예측: ");
  if (predicted_class == 1)
    Serial.println("사람 접근!");
  else
    Serial.println("정상");
  
  delay(1000);
}
