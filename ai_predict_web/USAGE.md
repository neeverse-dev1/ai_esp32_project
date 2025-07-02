# AI Predict Web 사용법

## 1. esp32 arduino ide 실행

(1) ai_predict_web 폴더에 ai_predict_web.ino 을 더블클릭  또는 파일을 열어 실행한다

(2) wifi 설정 
* config.h 파일에서 ssid, password 입력

```c++
// config.h

// 중간 생략

// --- Wi-Fi 설정 ---
const char* ssid = "xxxxxxx";     // Wi-Fi 이름 (SSID)을 여기에 입력하세요
const char* password = "xxxxxxxx"; // Wi-Fi 비밀번호를 여기에 입력하세요

#endif // CONFIG_H
```

(2) Arduino IDE 에서 uplaod 버튼을 누르고 컴파일 및 업로드를 하여 실행 

* 업로드가 완료 되면 Serial Monitor 에 할당받은 IP를 확인하고 웹브라우져 주소창에 입력해서 모니터링 페이지 확인

* 다음은 정상 동작할 경우 시리얼 모니터에 출력된다.
```c++
Wi-fi 연결성공 !
ESP32 IP 주소: x.x.x.x
ESP32 MAC 주소: xx:xx:xx:xx:xx:xx
HTTP 웹 서버 시작됨.
25.1    41.30   10
.
. 
```

(3) Serial Plotter로 plot 차트 확인


(4) 웹 브라우져 접속 

* http://{할당받은 IP주소}


<img src="https://github.com/neeverse-dev1/ai_esp32_project/blob/main/images/esp32_web.png" width="100%" height="100%"/>

(5) Arduino IDE 프로그램 종료

* ai predict 실행 시키기 위함

## 2. AI 예측 (Python)


(1) python으로 ai 학습 및 실행하기 위해서는 package가 필요
* 다음과 같이 git bash 에서 순차적으로 실행

```c
python.exe -m pip install --upgrade pip
pip install pyserial requests pandas scikit-learn

```

(2) ai_predictor.py 설정값 변경

* 시리얼포트, ESP32 IP주소 설정 (메모장 또는 vscode)

```python
# --- 설정값 --ㅛ
SERIAL_PORT = 'COM5'
BAUD_RATE = 115200
ESP32_IP = 'x.x.x.x'
```

(3) ai_preditcor.py 실행

* 최초 실행시 에러 발생 함 : 학습된 model 파일이 없을 경우 벌생
* 1분정도 실행 하여 sensor_data_log.csv 자동 생성 하게 실행 (로그파일은 많을수록 예측도 상승됨)

```
$ python ai_predictor.py

경고: 학습된 모델 파일(temp_model.pkl, hum_model.pkl, dist_model.pkl)을 찾을 수 없습니다.
먼저 train_model.py 스크립트를 실행하여 모델을 학습시키고 저장해야 합니다.
현재는 예측이 정확하지 않을 수 있습니다. (임시 모델 사용)
시리얼 포트 COM5에 연결되었습니다.
수신 (센서 데이터): T=29.10, H=42.00, D=201
수신 (센서 데이터): T=29.10, H=42.00, D=201
수신 (센서 데이터): T=29.10, H=42.00, D=201

```

(4) 센서 데이터 모델 트레이닝
* train_model.py 실행 시켜 log값을 센서별로 트레이닝

```
$ python train_model.py

Temperature MAE: 0.00
Humidity MAE: 0.00
Distance MAE: 0.00
모델 학습 및 저장 완료.
```

(5) 트레이닝 된 모델로 ai_predictor.py 재실행
* 실시간 데이터를 폴링하여 예측값 자동 업데이트
* 웹 브라우저에서 예측값 자동 업데이트 확인 

```
$ python ai_predictor.py

AI 모델 로드 완료.
시리얼 포트 COM5에 연결되었습니다.
수신 (센서 데이터): T=29.40, H=43.00, D=201
수신 (센서 데이터): T=29.40, H=43.00, D=201
수신 (센서 데이터): T=29.40, H=43.00, D=201
수신 (센서 데이터): T=29.40, H=43.00, D=201
수신 (센서 데이터): T=29.40, H=43.00, D=201
수신 (센서 데이터): T=29.40, H=43.00, D=201
수신 (센서 데이터): T=29.40, H=43.00, D=201
수신 (센서 데이터): T=29.40, H=43.00, D=201
수신 (센서 데이터): T=29.40, H=43.00, D=201
수신 (센서 데이터): T=29.40, H=43.00, D=201
수신 (센서 데이터): T=29.40, H=43.00, D=201
수신 (센서 데이터): T=29.40, H=43.00, D=201
수신 (센서 데이터): T=29.40, H=43.00, D=201
수신 (센서 데이터): T=29.40, H=43.00, D=201
수신 (센서 데이터): T=29.40, H=43.00, D=201
예측값: T=29.2, H=42.0, D=201.0
예측값 전송 성공: {'temp_pred': np.float64(29.2), 'hum_pred': np.float64(42.0), 'dist_pred': 201} -> Prediction received
수신 (센서 데이터): T=29.40, H=43.00, D=201
수신 (센서 데이터): T=29.40, H=43.00, D=201
수신 (센서 데이터): T=29.40, H=43.00, D=201
수신 (센서 데이터): T=29.40, H=43.00, D=201
수신 (센서 데이터): T=29.40, H=43.00, D=201
수신 (센서 데이터): T=29.40, H=43.00, D=201
수신 (센서 데이터): T=29.40, H=43.00, D=201
수신 (센서 데이터): T=29.40, H=43.00, D=201

```

(6) 웹브라우져에서 AI 예측 값 확인 

(*) AI 모델 트레이닝
* sensor data 가 많을수록 좋
* ai_predictor.py 실행을 멈추고(Ctrl+c) train_model.py 재실행 
* 또는 터미널(git bash)을 새로 열어 train_model.py 실행
* pkl 파일, log파일 오버라이딩 됨


<img src="https://github.com/neeverse-dev1/ai_esp32_project/blob/main/images/esp32_web_predictor_run.png" width="100%" height="100%"/>