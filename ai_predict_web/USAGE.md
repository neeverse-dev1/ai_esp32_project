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


<img src="https://github.com/neeverse-dev1/ai_esp32_project/blob/main/images/esp32_web.png" width="480" height="320"/>

## 2. AI 예측 (Python)