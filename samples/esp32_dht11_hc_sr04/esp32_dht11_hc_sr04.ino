#include <DHT11.h>

#define DHTPIN 4
DHT11 dht11(DHTPIN);

#define trigPin   17
#define echoPin   16
long duration;
int distanceCm;

void setup()
{
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);

    Serial.begin(115200);
    Serial.println("ESP32 DHT11 and HC-SR04 Sensor Test");
    
    delay(2000);    
}

void loop()
{
    float h = dht11.readHumidity();
    float t = dht11.readTemperature();

    if (isnan(h) || isnan(t))
    {
        Serial.println("DHT 센서에서 데이터를 읽는 데 실패했습니다!");
        return;
    }

    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    duration = pulseIn(echoPin, HIGH);
    distanceCm = duration * 0.0343 / 2;
    if(distanceCm >= 200)
    {
        distanceCm = 200;
    }
    else if(distanceCm == 0)
    {
        Serial.println("Distance: Out of range or sensor error!");
    }

    Serial.print("습도:");  Serial.print(h);  Serial.print(",");
    Serial.print("온도:");  Serial.print(t);  Serial.print(",");
    Serial.print("거리:");  Serial.print(distanceCm);  Serial.print(",");
    Serial.println();
    
    delay(10);
}
