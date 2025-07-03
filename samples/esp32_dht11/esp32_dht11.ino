#include <DHT11.h>

#define DHTPIN 4 
DHT11 dht11(DHTPIN);

void setup()
{
    Serial.begin(115200);
    Serial.println("DHT11 테스트!");

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

    Serial.print("습도: ");    Serial.print(h);
    Serial.print("%  온도: "); Serial.print(t);
    Serial.println("°C");
    delay(100);
}
