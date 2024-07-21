// Define Blynk parameters according to the project settings
#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "your template id here"
#define BLYNK_TEMPLATE_NAME "your template name here"
#define BLYNK_AUTH_TOKEN "your auth token here"

// Include necessary libraries for WiFi and Blynk
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Authentication information for wifi network to be used to connect to the blynk server
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "your wifi hotspot name here";  // WiFi SSID
char pass[] = "your wifi password here";      // WiFi Password

// Include the ESP32 Servo library
#include <ESP32Servo.h>
Servo servo;  // Create servo object

// Variables for ultrasonic sensor
long duration;
int distance;
int binLevel=0;

// Define pin numbers
#define echoPin 32  // Echo pin for the ultrasonic sensor
#define trigPin 33  // Trigger pin for the ultrasonic sensor
#define irPin 34    // IR sensor pin
#define servoPin 13 // Servo control pin

// Variable to track Blynk connection status
bool blynkConnected = false;

// Function to measure distance using ultrasonic sensor
void ultrasonic() {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    distance = duration * 0.034 / 2;
    binLevel = map(distance, 21, 0, 0, 100);  // Map distance to bin level percentage
    // Adjust the first two parameters of the map function according to your dustbin depth
    // Example: If your dustbin is 30 cm deep, use map(distance, 30, 0, 0, 100);


    // Print distance and bin level to Serial Monitor
    Serial.print("Ultrasonic - Distance: ");
    Serial.print(distance);
    Serial.print(" cm, Bin Level: ");
    Serial.print(binLevel);
    Serial.println(" %");

    // Send data to Blynk if connected
    if (blynkConnected) {
        Blynk.virtualWrite(V0, distance);
        Blynk.virtualWrite(V1, binLevel);
    }
}

// Function to check IR sensor and control servo
void SMESensor() {
    int ir = digitalRead(irPin);
    Serial.print("IR Sensor Reading: ");
    Serial.println(ir == LOW ? "Object Detected" : "No Object Detected");

    // Control servo based on IR sensor reading
    if (ir == LOW) {
        servo.write(90);  // Open bin lid
        Serial.println("Servo Angle: 90 (Bin lid opened)");
    } else {
        servo.write(0);  // Close bin lid
        Serial.println("Servo Angle: 0 (Bin lid closed)");
    }

    // Send servo position to Blynk if connected
    if (blynkConnected) {
        Blynk.virtualWrite(V2, servo.read());
    }
}

// Setup function
void setup() {
    Serial.begin(9600);  // Start serial communication
    servo.attach(servoPin);  // Attach servo to its control pin
    pinMode(irPin, INPUT);  // Set IR pin as input
    pinMode(trigPin, OUTPUT);  // Set trigger pin as output
    pinMode(echoPin, INPUT);  // Set echo pin as input

    // Attempt to connect to WiFi
    WiFi.begin(ssid, pass);
    long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) { // 10-second timeout
        delay(500);
        Serial.print(".");
    }
    // Attempt to connect to Blynk
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

// Main loop function
void loop() {
    static unsigned long lastSensorRead = 0;
    if (millis() - lastSensorRead > 1000) { // Non-blocking delay
        lastSensorRead = millis();
        SMESensor();
        ultrasonic();
    }

    // Run Blynk if connected
    if (blynkConnected) {
        Blynk.run();
    }
}