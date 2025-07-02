// config.h

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h> // String 타입을 위해 필요할 수 있습니다.

// --- 분석을 위한 임계값 설정 (const 제거) ---
// 센서 데이터 분석 기준이 되는 값들입니다.
extern float HIGH_TEMP_THRESHOLD;     // 고온 경고 임계값 (섭씨)
extern float HIGH_HUMIDITY_THRESHOLD; // 고습 경고 임계값 (%)
extern int OBJECT_CLOSE_THRESHOLD;      // 물체 근접 경고 임계값 (cm)
extern int OBJECT_VERY_CLOSE_THRESHOLD; // 물체 매우 근접 경고 임계값 (cm)

// --- 전역 변수로 센서 값 저장 ---
// 현재 센서 값과 분석 상태를 저장하는 변수들입니다.
//extern float currentTemp;     // 현재 온도
//extern float currentHum;      // 현재 습도
//extern int currentDistance;   // 현재 거리
//extern String currentStatus;  // 현재 상태 메시지

// --- Wi-Fi 설정 ---
const char* ssid = "mkHome_2.4Ghz";     // Wi-Fi 이름 (SSID)을 여기에 입력하세요
const char* password = "mk800130"; // Wi-Fi 비밀번호를 여기에 입력하세요

#endif // CONFIG_H
