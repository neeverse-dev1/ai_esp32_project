# AI Predict Web 연동

## 1. 환경설정

> arduino ide 패키지 설치 및 python 환경구성이 필요합니다.

### (1) Arduino IDE 필요 패키지 설치

* **ArduinoJson 설치:** ArduinoJson 검색
* **NewPing 설치:** NewPing 검색 (hc_sr04 관련 라이브러리)

### (2) Python 환경구성

1. python 설치 
- Python 다운로드 페이지 (https://www.python.org/downloads/)
- https://www.python.org/ftp/python/3.12.9/python-3.12.9-amd64.exe (python 버전 : 3.12 추천)

<img src="https://github.com/neeverse-dev1/ai_esp32_project/blob/main/images/python312_install_0.png" width="480" height="320"/>

> add python.exe to path 체크 (시스템 환경변수 등록)

<img src="https://github.com/neeverse-dev1/ai_esp32_project/blob/main/images/python312_install_2.png" width="480" height="320"/>

<img src="https://github.com/neeverse-dev1/ai_esp32_project/blob/main/images/python312_install_3.png" width="480" height="320"/>


### (3) git bash 설치 및 파이썬 설치 확인

1. git bash 설치 
- git bash 다운 페이지 https://git-scm.com/

> 참고 : https://velog.io/@selenium/Git-Git-Bash-%EC%84%A4%EC%B9%98-Windows-OS

2. git bash 실행
- python 설치 및 버전 확인

<img src="https://github.com/neeverse-dev1/ai_esp32_project/blob/main/images/git_bash_python_check.png" width="480"ㅛheight="320"/>

## 2. git bash (터미널) 이용하여 소스 다운로드 및 실행

- 탐색기에서 c: 드라이브로 이동해서 마우스 오른쪽 버튼 누르고 "git bash" 선택하면 path가 c:/로 지정됨

<img src="https://github.com/neeverse-dev1/ai_esp32_project/blob/main/images/git_bash_c_drive.png" width="480"ㅛheight="320"/>

### (4) 샘플소스 다운로드 

> git clone 하여 샘플소스 다운로드

```
git clone https://github.com/neeverse-dev1/ai_esp32_project.git
```

<img src="https://github.com/neeverse-dev1/ai_esp32_project/blob/main/images/git_bash_clone_repo.png" width="480"ㅛheight="320"/>

