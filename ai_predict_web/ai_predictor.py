import serial
import time
import requests
import json
import pandas as pd
from sklearn.linear_model import LinearRegression # 예시 모델, 더 복잡한 모델 사용 가능

# --- 설정값 ---
SERIAL_PORT = 'COM5'  # ESP32가 연결된 시리얼 포트 (Windows: COMx, Mac/Linux: /dev/ttyUSBx 또는 /dev/ttyACMx)
BAUD_RATE = 115200
ESP32_IP = '192.168.0.65' # ESP32의 실제 IP 주소로 변경하세요!
PREDICT_INTERVAL = 10 # 예측 후 ESP32로 전송하는 간격 (초)

# --- AI 모델 초기화 (예시: 간단한 선형 회귀) ---
# 실제 데이터로 학습시켜야 합니다. 여기서는 임시 모델입니다.
# 더 복잡한 시계열 예측 모델 (ARIMA, LSTM 등)을 사용할 수 있습니다.
temp_model = LinearRegression()
hum_model = LinearRegression()
dist_model = LinearRegression()

# 모델 학습을 위한 임시 데이터 (실제 센서 데이터로 대체해야 함)
# 시계열 예측을 위해선 이전 N개의 데이터를 입력으로 사용합니다.
# 여기서는 가장 간단하게 시간 인덱스를 입력으로 사용합니다.
train_data_count = 100
X_train = [[i] for i in range(train_data_count)]
y_temp_train = [25 + i*0.1 + (i%10)/2 for i in range(train_data_count)]
y_hum_train = [60 + i*0.2 - (i%5)/3 for i in range(train_data_count)]
y_dist_train = [50 - i*0.5 + (i%20) for i in range(train_data_count)]

temp_model.fit(X_train, y_temp_train)
hum_model.fit(X_train, y_hum_train)
dist_model.fit(X_train, y_dist_train)

print("AI 모델 학습 완료 (임시 데이터)")

# --- 센서 데이터 저장 버퍼 ---
# 예측을 위해 일정량의 이전 데이터를 저장합니다.
# 예측 모델에 따라 이 버퍼의 크기와 사용 방식이 달라질 수 있습니다.
sensor_data_buffer = {
    'temperature': [],
    'humidity': [],
    'distance': []
}
BUFFER_SIZE = 20 # 예측에 사용할 데이터 포인트 수

# --- 시리얼 포트 연결 ---
try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    print(f"시리얼 포트 {SERIAL_PORT}에 연결되었습니다.")
except serial.SerialException as e:
    print(f"시리얼 포트 연결 실패: {e}")
    print("올바른 시리얼 포트와 ESP32가 연결되어 있는지 확인하세요.")
    exit()

# --- ESP32로 예측값 전송 함수 ---
def send_predictions_to_esp32(temp_pred, hum_pred, dist_pred):
    url = f"http://{ESP32_IP}/predict"
    headers = {'Content-Type': 'application/json'}
    payload = {
        'temp_pred': round(temp_pred, 1),
        'hum_pred': round(hum_pred, 1),
        'dist_pred': int(dist_pred)
    }
    try:
        response = requests.post(url, headers=headers, data=json.dumps(payload))
        response.raise_for_status() # HTTP 오류 발생 시 예외 발생
        print(f"예측값 전송 성공: {payload} -> {response.text}")
    except requests.exceptions.RequestException as e:
        print(f"예측값 전송 실패: {e}")
        print(f"ESP32 IP: {ESP32_IP}가 올바른지, ESP32 웹 서버가 실행 중인지 확인하세요.")

# --- 메인 루프 ---
last_predict_time = time.time()
data_counter = 0 # 임시 모델 학습을 위한 데이터 카운터

while True:
    try:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').strip()
            print(f"수신: {line}")

            parts = line.split() # 공백으로 분리
            if len(parts) == 3:
                try:
                    temp = float(parts[0])
                    hum = float(parts[1])
                    dist = int(parts[2])

                    # 버퍼에 데이터 추가
                    sensor_data_buffer['temperature'].append(temp)
                    sensor_data_buffer['humidity'].append(hum)
                    sensor_data_buffer['distance'].append(dist)

                    # 버퍼 크기 유지
                    if len(sensor_data_buffer['temperature']) > BUFFER_SIZE:
                        sensor_data_buffer['temperature'].pop(0)
                        sensor_data_buffer['humidity'].pop(0)
                        sensor_data_buffer['distance'].pop(0)
                    
                    data_counter += 1 # 예측 모델 학습을 위한 카운터 증가

                    # 예측 및 전송 (일정 간격마다)
                    if time.time() - last_predict_time >= PREDICT_INTERVAL and \
                       len(sensor_data_buffer['temperature']) == BUFFER_SIZE: # 버퍼가 충분히 차면 예측
                        
                        # --- AI 예측 로직 (현재 간단한 선형 회귀) ---
                        # 실제로는 sensor_data_buffer의 데이터를 사용하여 다음 값을 예측합니다.
                        # 예시: 다음 시간 스텝의 온도를 예측
                        next_time_step = data_counter + 1 
                        pred_temp = temp_model.predict([[next_time_step]])[0]
                        pred_hum = hum_model.predict([[next_time_step]])[0]
                        pred_dist = dist_model.predict([[next_time_step]])[0]
                        
                        # 예측값 범위 보정 (예: 습도는 0-100%, 거리는 음수가 될 수 없음)
                        pred_temp = max(5.0, min(50.0, pred_temp)) # 예시: 온도 범위 5~50도
                        pred_hum = max(0.0, min(100.0, pred_hum)) # 습도 0~100%
                        pred_dist = max(0, pred_dist) # 거리 음수 방지

                        print(f"예측값: T={pred_temp:.1f}, H={pred_hum:.1f}, D={pred_dist}")
                        send_predictions_to_esp32(pred_temp, pred_hum, pred_dist)
                        last_predict_time = time.time()
                
                except ValueError:
                    print(f"숫자 변환 오류: {line}")
            else:
                print(f"잘못된 형식의 데이터: {line}")

    except serial.SerialTimeoutException:
        print("시리얼 타임아웃 발생")
    except Exception as e:
        print(f"예기치 않은 오류 발생: {e}")
        break # 오류 발생 시 루프 종료
    
    time.sleep(0.1) # CPU 과부하 방지


