import serial
import csv
import time

# 아두이노와 연결된 시리얼 포트 설정
# 자신의 환경에 맞게 'COM3' 부분을 수정해주세요.
# Windows: 'COMx' (예: 'COM3', 'COM4')
# macOS/Linux: '/dev/ttyUSBx' 또는 '/dev/ttyACMx' (예: '/dev/ttyUSB0', '/dev/ttyACM0')
SERIAL_PORT = 'COM5' 
BAUD_RATE = 115200

# CSV 파일 이름 설정
CSV_FILENAME = 'sensor_data.csv'

# 데이터 수집 시간 (초) 또는 특정 조건까지
# 예를 들어, 60초 동안 데이터를 수집하려면 DURATION = 60
# 무한정 수집하려면 DURATION을 주석 처리하거나 매우 큰 값으로 설정
DURATION = 60 # 60초 동안 데이터 수집 예시

def read_and_save_data():
    try:
        # 시리얼 포트 열기
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        print(f"시리얼 포트 {SERIAL_PORT}가 열렸습니다.")
        time.sleep(2) # 아두이노 초기화 시간 대기

        with open(CSV_FILENAME, 'w', newline='') as csvfile:
            csv_writer = csv.writer(csvfile)
            # CSV 헤더 작성 (아두이노에서 출력되는 순서에 맞게)
            csv_writer.writerow(['temp', 'hum', 'distance', 'label'])
            print(f"'{CSV_FILENAME}' 파일에 데이터를 저장합니다.")

            start_time = time.time()
            # while True: # 무한 루프 (DURATION을 사용하지 않을 경우)
            while (time.time() - start_time) < DURATION:
                if ser.in_waiting > 0:
                    line = ser.readline().decode('utf-8').strip()
                    if line:
                        try:
                            # 쉼표로 구분된 문자열을 분리하여 리스트로 변환
                            data = [float(x) if i < 3 else int(x) for i, x in enumerate(line.split(','))]
                            csv_writer.writerow(data)
                            print(f"저장된 데이터: {data}")
                        except ValueError as e:
                            print(f"데이터 파싱 오류: {e}, 라인: {line}")
                time.sleep(0.1) # CPU 사용률을 줄이기 위한 짧은 대기

    except serial.SerialException as e:
        print(f"시리얼 포트 오류: {e}")
        print("올바른 시리얼 포트를 설정했는지, 아두이노가 연결되어 있는지 확인하세요.")
    except Exception as e:
        print(f"예상치 못한 오류 발생: {e}")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print("시리얼 포트가 닫혔습니다.")

if __name__ == "__main__":
    read_and_save_data()

