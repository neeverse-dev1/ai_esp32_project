#ifndef WEB_PAGE_H
#define WEB_PAGE_H

// 웹 페이지 HTML/JavaScript 내용을 PROGMEM에 저장
const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="ko">
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta charset="UTF-8">
<title>ESP32 AI 센서 모니터링</title>
<link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css" rel="stylesheet">
<script src="https://cdn.jsdelivr.net/npm/chart.js@3.7.0/dist/chart.min.js"></script> 
<script src="https://cdn.jsdelivr.net/npm/chartjs-plugin-annotation@2.2.1/dist/chartjs-plugin-annotation.umd.min.js"></script>
<style>
  body { font-family: Arial, sans-serif; text-align: center; margin: 0; padding: 20px; background-color: #f8f9fa; color: #333; }
  .container { background-color: #fff; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); padding: 20px; margin-top: 20px; }
  h1 { color: #0d6efd; text-align: left; margin-bottom: 20px; }
  .data-item { padding: 10px 0; border-bottom: 1px solid #eee; text-align: left; }
  .data-item:last-child { border-bottom: none; }
  .label { font-weight: bold; color: #555; margin-right: 10px; }
  .value { font-size: 1.1em; color: #198754; }
  .status { font-size: 1.2em; font-weight: bold; padding: 8px 15px; border-radius: 5px; margin-top: 15px; text-align: center; }
  .status.normal { background-color: #d1e7dd; color: #0f5132; } 
  .status.warning { background-color: #fff3cd; color: #856404; } 
  .status.danger { background-color: #f8d7da; color: #721c24; }   
  .refresh-btn { background-color: #0d6efd; color: white; padding: 10px 15px; border: none; border-radius: 5px; cursor: pointer; margin-top: 20px; }
  .refresh-btn:hover { background-color: #0b5ed7; }
  .chart-container { width: 100%; margin-top: 20px; }
  /* 임계값 설정 섹션 스타일 */
  .threshold-section { margin-top: 30px; padding-top: 20px; border-top: 1px solid #eee; text-align: left;}
  .threshold-section h2 { color: #0d6efd; margin-bottom: 15px; }
  .threshold-item { margin-bottom: 10px; }
  .threshold-item label { font-weight: bold; display: inline-block; width: 120px; }
  .threshold-item input { width: 80px; padding: 5px; border: 1px solid #ccc; border-radius: 4px; }
  .save-threshold-btn { background-color: #28a745; color: white; padding: 10px 15px; border: none; border-radius: 5px; cursor: pointer; margin-top: 15px; }
  .save-threshold-btn:hover { background-color: #218838; }
</style>
</head>
<body>
<div class="container-fluid">
  <h1>ESP32 AI 센서 모니터링</h1>
  <div class="row">
    <div class="col-md-2">
      <div class="data-item"><span class="label">현재 시간:</span> <span id="current-time" class="value">--:--:--</span></div>
      <div class="data-item"><span class="label">온도:</span> <span id="temperature" class="value">--.- °C</span></div>
      <div class="data-item"><span class="label">습도:</span> <span id="humidity" class="value">--.- %</span></div>
      <div class="data-item"><span class="label">거리:</span> <span id="distance" class="value">--- cm</span></div>
      
      <hr> <div class="data-item"><span class="label">예측 온도:</span> <span id="predicted-temperature" class="value">--.- °C (AI)</span></div>
      <div class="data-item"><span class="label">예측 습도:</span> <span id="predicted-humidity" class="value">--.- % (AI)</span></div>
      <div class="data-item"><span class="label">예측 거리:</span> <span id="predicted-distance" class="value">--- cm (AI)</span></div>

      <div id="status-message" class="status normal">상태: 정상</div>
      <button class="refresh-btn" onclick="updateData()">데이터 새로고침</button>

      <div class="threshold-section">
        <h2>임계값 설정</h2>
        <div class="threshold-item">
          <label for="tempThreshold">고온 (°C):</label>
          <input type="number" id="tempThreshold" step="0.1">
        </div>
        <div class="threshold-item">
          <label for="humThreshold">고습 (%):</label>
          <input type="number" id="humThreshold" step="0.1">
        </div>
        <div class="threshold-item">
          <label for="closeThreshold">근접 (cm):</label>
          <input type="number" id="closeThreshold" step="1">
        </div>
        <div class="threshold-item">
          <label for="veryCloseThreshold">매우 근접 (cm):</label>
          <input type="number" id="veryCloseThreshold" step="1">
        </div>
        <button class="save-threshold-btn" onclick="saveThresholds()">임계값 저장</button>
      </div>

    </div>
    <div class="col-md-5">
      <div class="chart-container">
        <h2>온도 변화</h2>
        <canvas id="tempChart"></canvas>
      </div>
      <div class="chart-container">
        <h2>습도 변화</h2>
        <canvas id="humChart"></canvas>
      </div>
    </div>
    <div class="col-md-5">
      <div class="chart-container">
        <h2>거리 변화</h2>
        <canvas id="distChart"></canvas>
      </div>
    </div>
  </div>
</div>

<script>
// --- 전역 스코프에 정의된 변수들 ---
const MAX_DATA_POINTS = 15;
let labels = [];
let tempData = [];
let humData = [];
let distData = [];
let tempChart, humChart, distChart;

// 임계값 변수를 전역으로 선언합니다.
let currentTempThreshold = 28.0;
let currentHumThreshold = 70.0;
let currentCloseThreshold = 30;
let currentVeryCloseThreshold = 10;

// --- 전역 스코프에 정의된 함수들 (JavaScript functions) ---

// 센서 데이터 업데이트 함수 (예측값 표시 로직 추가)
var updateData = () => {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var data = JSON.parse(this.responseText);
      
      document.getElementById("temperature").innerHTML = data.temperature.toFixed(1) + " °C";
      document.getElementById("humidity").innerHTML = data.humidity.toFixed(1) + " %";
      document.getElementById("distance").innerHTML = data.distance + " cm";

      // 예측값 표시 업데이트
      document.getElementById("predicted-temperature").innerHTML = data.predictedTemperature.toFixed(1) + " °C (AI)";
      document.getElementById("predicted-humidity").innerHTML = data.predictedHumidity.toFixed(1) + " % (AI)";
      document.getElementById("predicted-distance").innerHTML = data.predictedDistance + " cm (AI)";

      var statusMessageElement = document.getElementById("status-message");
      statusMessageElement.innerHTML = "상태: " + data.status;
      
      statusMessageElement.className = "status";
      if (data.status.includes("위험")) {
        statusMessageElement.classList.add("danger");
      } else if (data.status.includes("경고") || data.status.includes("감지")) {
        statusMessageElement.classList.add("warning");
      } else {
        statusMessageElement.classList.add("normal");
      }

      const now = new Date();
      const timeLabel = now.toLocaleTimeString();

      labels.push(timeLabel);
      tempData.push(data.temperature);
      humData.push(data.humidity);
      distData.push(data.distance);

      // 예측값도 차트에 추가하고 싶다면 여기에 추가 로직을 넣을 수 있습니다.
      // 예: predictedTempData.push(data.predictedTemperature);

      if (labels.length > MAX_DATA_POINTS) {
        labels.shift();
        tempData.shift();
        humData.shift();
        distData.shift();
      }

      // 차트가 정의되어 있는지 확인 후 업데이트
      if (tempChart) tempChart.update();
      if (humChart) humChart.update();
      if (distChart) distChart.update();
    }
  };
  xhttp.open("GET", "/data", true);
  xhttp.send();
};


// 현재 시간을 업데이트하는 함수
var updateCurrentTime = () => {
    const now = new Date();
    const timeString = now.toLocaleTimeString();
    document.getElementById("current-time").innerHTML = timeString;
};

// 임계값 저장 함수 (웹 UI용)
var saveThresholds = () => {
  var temp = document.getElementById("tempThreshold").value;
  var hum = document.getElementById("humThreshold").value;
  var close = document.getElementById("closeThreshold").value;
  var veryClose = document.getElementById("veryCloseThreshold").value;

  var xhttp = new XMLHttpRequest();
  xhttp.open("POST", "/setThresholds", true);
  xhttp.setRequestHeader("Content-Type", "application/json"); 
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4) { 
      if (this.status == 200) {
        alert("임계값이 저장되었습니다!");
        getThresholds(); // 저장 후 현재 임계값 다시 불러와서 UI 및 차트 업데이트
      } else {
        alert("임계값 저장 실패: " + this.status + " " + this.statusText);
      }
    }
  };
  var params = JSON.stringify({ 
      temp: parseFloat(temp), 
      hum: parseFloat(hum), 
      close: parseInt(close), 
      veryClose: parseInt(veryClose)
  });
  xhttp.send(params);
};

// 임계값 불러오기 함수 (웹 UI용)
var getThresholds = () => {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var thresholds = JSON.parse(this.responseText);
      document.getElementById("tempThreshold").value = thresholds.temp;
      document.getElementById("humThreshold").value = thresholds.hum;
      document.getElementById("closeThreshold").value = thresholds.close;
      document.getElementById("veryCloseThreshold").value = thresholds.veryClose;

      // 불러온 임계값을 전역 변수에 저장합니다.
      currentTempThreshold = thresholds.temp;
      currentHumThreshold = thresholds.hum;
      currentCloseThreshold = thresholds.close;
      currentVeryCloseThreshold = thresholds.veryClose;

      // 임계값이 변경될 때마다 차트를 업데이트하여 선을 다시 그립니다.
      // 차트가 초기화되어 있지 않으면 (초기 로드 시) 바로 업데이트하지 않습니다.
      if (tempChart) tempChart.destroy(); // 기존 차트 파괴 후 재초기화
      if (humChart) humChart.destroy();
      if (distChart) distChart.destroy();
      initCharts(); // 새 임계값으로 차트 재초기화

      // 임계값을 로드하고 차트가 초기화된 후, 첫 데이터를 가져와 표시합니다.
      updateData(); 
    }
  };
  xhttp.open("GET", "/getThresholds", true);
  xhttp.send();
};


// 차트 초기화 함수 (이중 임계값 선 추가)
var initCharts = () => {
  const ctxTemp = document.getElementById('tempChart').getContext('2d');
  tempChart = new Chart(ctxTemp, {
    type: 'line',
    data: {
      labels: labels,
      datasets: [{
        label: '온도 (°C)',
        data: tempData,
        borderColor: 'rgb(255, 99, 132)',
        tension: 0.1,
        fill: false
      }]
    },
    options: {
      responsive: true,
      animation: { duration: 0 },
      scales: {
        y: {
          beginAtZero: false
        }
      },
      
    }
  });

  const ctxHum = document.getElementById('humChart').getContext('2d');
  humChart = new Chart(ctxHum, {
    type: 'line',
    data: {
      labels: labels,
      datasets: [{
        label: '습도 (%)',
        data: humData,
        borderColor: 'rgb(54, 162, 235)',
        tension: 0.1,
        fill: false
      }]
    },
    options: {
      responsive: true,
      animation: { duration: 0 },
      scales: {
        y: {
          beginAtZero: true,
          max: 100
        }
      },
      
      plugins: {
        annotation: {
          annotations: {
            humThresholdLine1: {
              type: 'line',
              yMin: currentHumThreshold,
              yMax: currentHumThreshold,
              borderColor: 'rgb(255, 0, 0)',
              borderWidth: 2,
              borderDash: [5, 5],
              label: {
                content: '고습 임계값',
                enabled: true,
                position: 'end'
              }
            },
            humThresholdLine2: {
              type: 'line',
              yMin: currentHumThreshold + 1.0, // 첫 번째 선보다 약간 위 (조절 가능)
              yMax: currentHumThreshold + 1.0,
              borderColor: 'rgb(255, 0, 0)',
              borderWidth: 2,
              borderDash: [5, 5]
            }
          }
        }
      }
    }
  });

  const ctxDist = document.getElementById('distChart').getContext('2d');
  distChart = new Chart(ctxDist, {
    type: 'line',
    data: {
      labels: labels,
      datasets: [{
        label: '거리 (cm)',
        data: distData,
        borderColor: 'rgb(75, 192, 192)',
        tension: 0.1,
        fill: false
      }]
    },
    options: {
      responsive: true,
      animation: { duration: 0 },
      scales: {
        y: {
          beginAtZero: true,
          max: 200
        }
      },
      plugins: {
        annotation: {
          annotations: {
            distCloseThresholdLine1: {
              type: 'line',
              yMin: currentCloseThreshold,
              yMax: currentCloseThreshold,
              borderColor: 'rgb(255, 165, 0)', // 주황색
              borderWidth: 2,
              borderDash: [5, 5],
              label: {
                content: '근접 임계값',
                enabled: true,
                position: 'end'
              }
            },
            distCloseThresholdLine2: {
              type: 'line',
              yMin: currentCloseThreshold + 1, // 1cm 차이
              yMax: currentCloseThreshold + 1,
              borderColor: 'rgb(255, 165, 0)',
              borderWidth: 2,
              borderDash: [5, 5]
            },
            distVeryCloseThresholdLine1: {
              type: 'line',
              yMin: currentVeryCloseThreshold,
              yMax: currentVeryCloseThreshold,
              borderColor: 'rgb(255, 0, 0)', // 빨간색
              borderWidth: 2,
              borderDash: [5, 5],
              label: {
                content: '매우 근접 임계값',
                enabled: true,
                position: 'end'
              }
            },
            distVeryCloseThresholdLine2: {
              type: 'line',
              yMin: currentVeryCloseThreshold + 1, // 1cm 차이
              yMax: currentVeryCloseThreshold + 1,
              borderColor: 'rgb(255, 0, 0)',
              borderWidth: 2,
              borderDash: [5, 5]
            }
          }
        }
      }
    }
  });
};


// --- window.onload 블록 시작 (초기화 로직) ---
window.onload = function() {
  console.log("annotationPlugin is defined:", typeof annotationPlugin !== 'undefined');
  //Chart.register(annotationPlugin);
  // Chart.js annotation 플러그인 등록
  console.log("ChartjsPluginAnnotation is defined:", typeof ChartjsPluginAnnotation !== 'undefined');
  //Chart.register(ChartjsPluginAnnotation); // <-- 이 줄이 제대로 작동하는지 확인해야 합니다. (주석 해제됨)

  // 페이지 로드 시 임계값을 가져오고, 그 안에서 차트 초기화와 첫 updateData() 호출을 처리합니다.
  getThresholds();
  
  // 현재 시간 업데이트는 별도로 즉시 호출하고, 주기적으로 실행합니다.
  updateCurrentTime();
  setInterval(updateCurrentTime, 1000);

  // 센서 데이터 업데이트는 getThresholds() 내부에서 첫 호출 후, 주기적으로 실행합니다.
  setInterval(updateData, 3000); 
};

</script>
</body>
</html>
)=====";

#endif // WEB_PAGE_H