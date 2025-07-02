import requests
import csv
import json

# ESP32 웹 서버의 IP 주소 (시리얼 모니터에서 확인한 주소로 변경)
ESP32_IP = "192.168.x.x" 

# ESP32의 임계값 설정 API 엔드포인트
SET_THRESHOLDS_API_URL = f"http://{ESP32_IP}/setThresholds"
GET_THRESHOLDS_API_URL = f"http://{ESP32_IP}/getThresholds"

# CSV 파일 경로
CSV_FILE_PATH = "threshold_data.csv"

def get_current_thresholds():
    """현재 ESP32에 설정된 임계값을 가져옵니다."""
    try:
        response = requests.get(GET_THRESHOLDS_API_URL, timeout=5)
        response.raise_for_status() # HTTP 오류가 발생하면 예외 발생
        current_thresholds = response.json()
        print(f"현재 ESP32 임계값: {current_thresholds}")
        return current_thresholds
    except requests.exceptions.RequestException as e:
        print(f"현재 임계값을 가져오는 데 실패했습니다: {e}")
        return None

def update_thresholds_from_csv(csv_file):
    """
    CSV 파일에서 임계값을 읽어와 ESP32로 전송합니다.
    CSV 파일은 'temp,hum,close,veryClose' 헤더를 가진다고 가정합니다.
    """
    try:
        with open(csv_file, 'r', encoding='utf-8') as f:
            reader = csv.DictReader(f)
            # CSV 파일의 첫 번째 데이터 행만 읽음
            for row in reader:
                # 데이터 유효성 검사 및 타입 변환
                try:
                    threshold_data = {
                        "temp": float(row.get('temp', 0)),
                        "hum": float(row.get('hum', 0)),
                        "close": int(row.get('close', 0)),
                        "veryClose": int(row.get('veryClose', 0))
                    }
                except ValueError as e:
                    print(f"CSV 데이터 변환 오류: {e}. 유효한 숫자를 확인하세요.")
                    return False
                
                print(f"CSV에서 읽은 임계값: {threshold_data}")

                # JSON 형식으로 변환
                json_payload = json.dumps(threshold_data)
                
                # HTTP POST 요청 보내기
                headers = {'Content-Type': 'application/json'}
                response = requests.post(SET_THRESHOLDS_API_URL, data=json_payload, headers=headers, timeout=10)
                
                # 응답 확인
                response.raise_for_status() # HTTP 오류(4xx, 5xx)가 발생하면 예외 발생
                print(f"임계값 업데이트 성공! 응답: {response.text}")
                return True # 성공적으로 처리했으면 종료
            
            print("CSV 파일에 임계값 데이터가 없습니다.")
            return False

    except FileNotFoundError:
        print(f"오류: '{csv_file}' 파일을 찾을 수 없습니다.")
        return False
    except requests.exceptions.RequestException as e:
        print(f"ESP32에 임계값을 전송하는 데 실패했습니다: {e}")
        return False
    except Exception as e:
        print(f"예상치 못한 오류 발생: {e}")
        return False

if __name__ == "__main__":
    print("--- 임계값 업데이트 스크립트 시작 ---")
    
    # 현재 임계값 확인 (선택 사항)
    get_current_thresholds()

    # CSV 파일에서 임계값 업데이트
    success = update_thresholds_from_csv(CSV_FILE_PATH)
    
    if success:
        print("CSV 파일로 임계값 업데이트가 요청되었습니다.")
        # 업데이트된 임계값이 ESP32에 반영되었는지 다시 확인 (선택 사항)
        get_current_thresholds()
    else:
        print("임계값 업데이트에 실패했습니다.")

    print("--- 임계값 업데이트 스크립트 종료 ---")
