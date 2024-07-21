#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6cYStE1Hq"
#define BLYNK_TEMPLATE_NAME "smartDustbin"
#define BLYNK_AUTH_TOKEN "E1OcEuEqqPVYrwkS4rSNYGPKIRAmkPKA"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "MyPublicWiFi";
char pass[] = "12345678";

#include <ESP32Servo.h>
Servo servo;
long duration;
int distance; 
int binLevel=0;

#define echoPin 32
#define trigPin 33
#define irPin 34
#define servoPin 13

bool blynkConnected = false;

void ultrasonic() {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    distance = duration * 0.034 / 2;
    binLevel = map(distance, 21, 0, 0, 100);

    Serial.print("Ultrasonic - Distance: ");
    Serial.print(distance);
    Serial.print(" cm, Bin Level: ");
    Serial.print(binLevel);
    Serial.println(" %");

    if (blynkConnected) {
        Blynk.virtualWrite(V0, distance);
        Blynk.virtualWrite(V1, binLevel);
    }
}

void SMESensor() {
    int ir = digitalRead(irPin);
    Serial.print("IR Sensor Reading: ");
    Serial.println(ir == LOW ? "Object Detected" : "No Object Detected");

    if (ir == LOW) {
        servo.write(90);
        Serial.println("Servo Angle: 90 (Bin lid opened)");
    } else {
        servo.write(0);
        Serial.println("Servo Angle: 0 (Bin lid closed)");
    }

    if (blynkConnected) {
        Blynk.virtualWrite(V2, servo.read());
    }
}

void setup() {
    Serial.begin(9600);
    servo.attach(servoPin);
    pinMode(irPin, INPUT);
    pinMode(trigPin, OUTPUT); 
    pinMode(echoPin, INPUT); 

    WiFi.begin(ssid, pass);
    long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) { // 10-second timeout
        delay(500);
        Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) {
        Blynk.begin(auth, ssid, pass);
        if (Blynk.connected()) {
            blynkConnected = true;
            Serial.println("Connected to Blynk server");
        }
    }
    if (!blynkConnected) {
        Serial.println("Failed to connect to Blynk. Running in offline mode.");
    }
}

void loop() {
    static unsigned long lastSensorRead = 0;
    if (millis() - lastSensorRead > 1000) { // Non-blocking delay
        lastSensorRead = millis();
        SMESensor();
        ultrasonic();
    }

    if (blynkConnected) {
        Blynk.run();
    }
}