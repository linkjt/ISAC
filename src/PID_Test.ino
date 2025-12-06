// --- PIN DEFINITIONS ---
const int sensorPin = A0;
const int mosfetPin = 6;
const int buttonPin = 2;

// --- CONFIGURATION ---
const unsigned long cookTimeOat = 30*1000;
const float IdealTemp = 27.0;
const unsigned long SampleTime = 1000; 

// --- PID CONSTANTS ---
const double Kp = 12;
const double Ki = 4.885;
const double Kd = .50125;

// --- PID VARIABLES ---
float PreviousError = 0.0;
float TotalIntegral = 0.0;
int PWM_Value = 0; // Current power output 0-255

// --- TIMING VARIABLES ---
unsigned long startTime = 0;
unsigned long lastSampleTime = 0;
bool isCooking = false;

void setup() {
  pinMode(buttonPin, INPUT); 
  pinMode(sensorPin, INPUT);
  pinMode(mosfetPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {

  if (digitalRead(buttonPin) == HIGH && !isCooking) {

    isCooking = true;
    startTime = millis();
    lastSampleTime = millis();
    Serial.println("--- Cooking Started ---");
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
      
      float error = IdealTemp - CurrentT;
      Serial.print("Temprature:");
      Serial.print(CurrentT);
      Serial.print(",PWM:");
      Serial.println(PWM_Value);
      Serial.print(",Error:");
      Serial.println(error);
    }
  }
}

// Function to stop the process cleanly
void endCook() {
  isCooking = false;
  analogWrite(mosfetPin, 0); 
  //TotalIntegral = 0;         
  PreviousError = 0;
  Serial.println("--- Cooking Finished Total Error:---");
  Serial.println(TotalIntegral);
}

int calculatePID(float Temp, unsigned long TimeDelta) {
  float timeSecs = TimeDelta / 1000.0;

  float error = IdealTemp - Temp;
  
  // Proportional Term
  float proportional = Kp * error;
  
  // Integral Term (Anti-windup added)
  TotalIntegral += error * timeSecs;
  // Limiting the integral
  //if (TotalIntegral > 200) TotalIntegral = 200;
  //if (TotalIntegral < -200) TotalIntegral = -200;
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
  float sensorValue = analogRead(sensorPin);
  // Assuming 5V reference: (Value / 1024) * 5000mV / 10mV_per_degree_C
  float temperatureC = (sensorValue * 5.0 / 1024.0) * 100.0;
  
  return temperatureC;
}
