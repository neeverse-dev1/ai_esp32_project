import serial
import matplotlib.pyplot as plt
from collections import deque

SERIAL_PORT = 'COM5'
BAUD_RATE = 115200

ser = serial.Serial(SERIAL_PORT, BAUD_RATE)
plt.ion()

max_len = 50
temp_data = deque(maxlen=max_len)
hum_data = deque(maxlen=max_len)
dist_data = deque(maxlen=max_len)

fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(8, 6))

while True:
    try:
        line = ser.readline().decode('utf-8').strip()
        parts = line.split(',')
        if len(parts) >= 3:
            temp, hum, dist = map(float, parts[:3])
            temp_data.append(temp)
            hum_data.append(hum)
            dist_data.append(dist)

            ax1.clear(); ax2.clear(); ax3.clear()
            ax1.plot(temp_data, label='Temp (°C)', color='red')
            ax2.plot(hum_data, label='Humidity (%)', color='blue')
            ax3.plot(dist_data, label='Distance (cm)', color='green')

            for ax in [ax1, ax2, ax3]:
                ax.legend(loc='upper right')
                ax.grid(True)

            plt.pause(0.05)
    except KeyboardInterrupt:
        print("🛑 종료됨")
        break
