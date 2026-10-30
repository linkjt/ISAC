#include <WiFi.h>
#define MQTT_MAX_PACKET_SIZE 1024
#include <PubSubClient.h>
#include <ESP32Servo.h>
#include <ArduinoJson.h>

const char* ssid = "La Red";
const char* password = "smellthecedar";
const char* mqtt_server = "192.168.4.60";
 
// Add these global variables at the top
int dispenseOat = 0;
int dispenseRasin = 0;
int dispenseSugar = 0;
bool startNewCook = false;

// --- NetWork CONFIG
WiFiClient espClient;
PubSubClient client(espClient);

// --- PIN DEFINITIONS ---
const int sensorPin = 35;
const int mosfetPin = 17;
const int buttonPin = 15;

// --- CONFIGURATION ---
unsigned long cookTimeOat = 60 * 1000;
float IdealTemp = 100.0;
const unsigned long SampleTime = 1000;

// --- PID CONSTANTS ---
const double Kp = 1.0;
const double Ki = 0.0;
const double Kd = 0.0;

// --- PID VARIABLES ---
float PreviousError = 0.0;
float TotalIntegral = 0.0;
int PWM_Value = 0;

// --- TIMING VARIABLES ---
unsigned long startTime = 0;
unsigned long lastSampleTime = 0;
bool isCooking = false;

// --- SERVO VARIBLES ---
const int ServoSpeed = 90;
int ServoDelay =  2000;

Servo servos[5];
const int oat = 0;
const int rasin = 1;
const int sugar = 2;
int buttonInst = 0;
bool servoActive[5] = { false, false, false, false, false };

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("Attaching Pins");
  servos[oat].attach(25);
  servos[rasin].attach(26);
  servos[sugar].attach(27);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(sensorPin, INPUT);
  pinMode(mosfetPin, OUTPUT);
  Serial.println("Starting Wifi Connect");
  setup_wifi();
  Serial.println("Starting mqtt Connect");
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);


  analogWrite(mosfetPin, 0);
}

void setup_wifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32OatmealClient")) {
      Serial.println("Connected!");
      if (client.subscribe("oatmeal/commands")) {
        Serial.println("Subscribed to oatmeal/commands");
      } else {
        Serial.println("Failed to subscribe");
      }
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  // Check if a new cook was triggered by MQTT
  if (startNewCook) {
    startNewCook = false; // Reset the flag
    
    Serial.println("--- Dispensing Ingredients ---");
    dispense(oat, dispenseOat);
    dispense(rasin, dispenseRasin);
    dispense(sugar, dispenseSugar);
    
    isCooking = true;
    startTime = millis();
    lastSampleTime = millis();
    Serial.println("--- Cooking Started ---");
  }

  // Physical Button override
  if (digitalRead(buttonPin) == LOW && !isCooking) {
    isCooking = true;
    startTime = millis();
    lastSampleTime = millis();
    dispense(oat, 2000); 
    dispense(rasin, 2000);
  }

  if (isCooking) {
    unsigned long currentTime = millis();


    if (currentTime - startTime >= cookTimeOat) {
      endCook();
      return;
    }


    if (currentTime - lastSampleTime >= SampleTime) {

      unsigned long timeDelta = currentTime - lastSampleTime;

      float CurrentT = getTemprature();
      PWM_Value = calculatePID(CurrentT, timeDelta);

      analogWrite(mosfetPin, PWM_Value);
      lastSampleTime = currentTime;


      Serial.println("Temp: " + String(CurrentT) + ", PWM: " + String(PWM_Value));
    }
  }
}

// Function to stop the process cleanly
void endCook() {
  isCooking = false;
  analogWrite(mosfetPin, 0);
  TotalIntegral = 0;
  PreviousError = 0;
  Serial.println("--- Cooking Finished ---");
}

int calculatePID(float Temp, unsigned long TimeDelta) {
  float timeSecs = TimeDelta / 1000.0;

  float error = IdealTemp - Temp;

  // Proportional Term
  float proportional = Kp * error;

  // Integral Term (Anti-windup added)
  TotalIntegral += error * timeSecs;
  // Limiting the integral
  if (TotalIntegral > 200) TotalIntegral = 200;
  if (TotalIntegral < -200) TotalIntegral = -200;
  float integral = Ki * TotalIntegral;

  // Derivative Term
  float derivative = Kd * (error - PreviousError) / timeSecs;
  PreviousError = error;

  float rawPWM = proportional + integral + derivative;

  int PWM = (int)rawPWM + 200;

  if (PWM < 0) PWM = 0;
  if (PWM > 255) PWM = 255;

  return PWM;
}

float getTemprature() {
  //float sensorValue = analogRead(sensorPin);
  float temperatureC = (analogRead(sensorPin) *  3.3) / 4095 / 0.01;

  return temperatureC;
}

void dispense(int servo, int duration) {
  servoActive[servo] = true;
  servos[servo].write(90 + ServoSpeed);
  delay(duration);
  servos[servo].write(90);
  servoActive[servo] = false;
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("Message arrived from Mosquito");
  
  StaticJsonDocument<1024> doc; // Reduced size slightly for safety
  DeserializationError error = deserializeJson(doc, payload, length);

  if (error) {
    Serial.print("JSON Parse failed: ");
    Serial.println(error.c_str());
    return;
  }

  // Match the keys in your JSON (oat, sugar, raisin)
  if (doc.containsKey("temp")) IdealTemp = doc["temp"];
  if (doc.containsKey("time")) cookTimeOat = doc["time"].as<unsigned long>();
  
  // Store quantities to be handled in the loop
  dispenseOat = doc["Slot0"] | 0;     // The '| 0' provides a default value
  dispenseRasin = doc["Slot1"] | 0;
  dispenseSugar = doc["Slot2"] | 0;
  
  startNewCook = true; 
  Serial.println("Command Received & Parsed");
}