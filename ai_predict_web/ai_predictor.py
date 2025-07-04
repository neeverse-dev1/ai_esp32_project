import serial
import time
import requests
import json
import pandas as pd
import joblib # Import joblib for loading models
import csv

# --- 설정값 ---
SERIAL_PORT = 'COM4' 
BAUD_RATE = 115200
ESP32_IP = '172.30.1.20'
PREDICT_INTERVAL = 10 # 예측 후 ESP32로 전송하는 간격 (초)
LOG_FILE = 'sensor_data_log.csv' # File to save real sensor data

# --- AI 모델 로드 ---
try:
    temp_model = joblib.load('temp_model.pkl')
    hum_model = joblib.load('hum_model.pkl')
    dist_model = joblib.load('dist_model.pkl')
    print("AI 모델 로드 완료.")
except FileNotFoundError:
    print("경고: 학습된 모델 파일(temp_model.pkl, hum_model.pkl, dist_model.pkl)을 찾을 수 없습니다.")
    print("먼저 train_model.py 스크립트를 실행하여 모델을 학습시키고 저장해야 합니다.")
    print("현재는 예측이 정확하지 않을 수 있습니다. (임시 모델 사용)")
    # Fallback to dummy models if files not found (for initial testing, but not for accurate prediction)
    from sklearn.linear_model import LinearRegression
    temp_model = LinearRegression()
    hum_model = LinearRegression()
    dist_model = LinearRegression()
    # Dummy training (remove this once you have proper trained models)
    train_data_count = 100
    X_train = [[i] for i in range(train_data_count)]
    y_temp_train = [25 + i*0.1 + (i%10)/2 for i in range(train_data_count)]
    y_hum_train = [60 + i*0.2 - (i%5)/3 for i in range(train_data_count)]
    y_dist_train = [50 - i*0.5 + (i%20) for i in range(train_data_count)]
    temp_model.fit(X_train, y_temp_train)
    hum_model.fit(X_train, y_hum_train)
    dist_model.fit(X_train, y_dist_train)


# --- 센서 데이터 저장 버퍼 ---
sensor_data_buffer = {
    'temperature': [],
    'humidity': [],
    'distance': []
}
BUFFER_SIZE = 5 # This MUST match N_LAGS from your training script!

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
        response.raise_for_status()
        print(f"예측값 전송 성공: {payload} -> {response.text}")
    except requests.exceptions.RequestException as e:
        print(f"예측값 전송 실패: {e}")
        print(f"ESP32 IP: {ESP32_IP}가 올바른지, ESP32 웹 서버가 실행 중인지 확인하세요.")

# --- 메인 루프 ---
last_predict_time = time.time()

with open(LOG_FILE, 'a', newline='') as f_log:
    csv_writer = csv.writer(f_log)
    if f_log.tell() == 0:
        csv_writer.writerow(['Timestamp', 'Temperature', 'Humidity', 'Distance'])

    while True:
        try:
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8').strip()
                
                parts = line.split()

                if len(parts) == 3:
                    try:
                        temp = float(parts[0])
                        hum = float(parts[1])
                        dist = int(parts[2])

                        print(f"수신 (센서 데이터): T={temp:.2f}, H={hum:.2f}, D={dist}")

                        timestamp = time.strftime("%Y-%m-%d %H:%M:%S")
                        csv_writer.writerow([timestamp, temp, hum, dist])
                        f_log.flush() 

                        sensor_data_buffer['temperature'].append(temp)
                        sensor_data_buffer['humidity'].append(hum)
                        sensor_data_buffer['distance'].append(dist)

                        if len(sensor_data_buffer['temperature']) > BUFFER_SIZE:
                            sensor_data_buffer['temperature'].pop(0)
                            sensor_data_buffer['humidity'].pop(0)
                            sensor_data_buffer['distance'].pop(0)
                        
                        # Only try to predict if the buffer is full (i.e., we have enough lagged data)
                        if time.time() - last_predict_time >= PREDICT_INTERVAL and \
                           len(sensor_data_buffer['temperature']) == BUFFER_SIZE: 
                            
                            # --- AI 예측 로직 (사용자 정의 예측 함수 사용) ---
                            # This is the critical change! You now use the buffer for prediction.
                            # The input to predict() must match the feature format used during training.
                            # For our LinearRegression with lagged features example:
                            input_features_temp = [sensor_data_buffer['temperature']]
                            input_features_hum = [sensor_data_buffer['humidity']]
                            input_features_dist = [sensor_data_buffer['distance']]

                            pred_temp = temp_model.predict(input_features_temp)[0]
                            pred_hum = hum_model.predict(input_features_hum)[0]
                            pred_dist = dist_model.predict(input_features_dist)[0]
                            
                            # 예측값 범위 보정
                            pred_temp = max(5.0, min(50.0, pred_temp))
                            pred_hum = max(0.0, min(100.0, pred_hum))
                            pred_dist = max(0, pred_dist) 

                            print(f"예측값: T={pred_temp:.1f}, H={pred_hum:.1f}, D={pred_dist}")
                            send_predictions_to_esp32(pred_temp, pred_hum, pred_dist)
                            last_predict_time = time.time()
                        
                    except ValueError:
                        pass 
                else:
                    pass 

        except serial.SerialTimeoutException:
            print("시리얼 타임아웃 발생")
        except Exception as e:
            print(f"예기치 않은 오류 발생: {e}")
            break
        
        time.sleep(0.1)
