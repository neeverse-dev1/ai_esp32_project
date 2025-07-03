#define trigPin   17
#define echoPin   16

long duration;
int distanceCm;
int distanceInch;

void setup()
{
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    Serial.begin(115200);
}

void loop()
{
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    duration = pulseIn(echoPin, HIGH);
    distanceCm = duration * 0.0343 / 2;
    if(distanceCm >= 200) distanceCm = 200;
    distanceInch = distanceCm / 2.54;

    Serial.print("Distance: ");   
    Serial.print(distanceCm);
    Serial.print(" cm");
    Serial.print(" | ");
    Serial.print(distanceInch);
    Serial.println(" inch");

    delay(100);
}
