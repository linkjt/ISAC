  #include <Servo.h>

const int sensorPin = A0;
const int mosfetPin = 6;
const int buttonPin = 2; 

const int cookTimeOat = 36;
const int IdealTemp = 100;

const double Kp = 1;
const double Ki =1;
const double Kd =1;
float PreviousError = 80;
int TotalIntergral = 0;

Servo servos[5];
const int oat = 0;
const int rasin = 1;
const int sugar = 2;
int buttonInst = 0;  
bool servoActive[5] = {false,false,false,false,false};
//attaches it to a random pin cause y not
void setup() {
  servos[oat].attach(9);
  servos[rasin].attach(10);
  servos[sugar].attach(11);
  pinMode(buttonPin, INPUT);
  pinMode(sensorPin, INPUT);
  pinMode(mosfetPin, OUTPUT);
  Serial.begin(9600);
}

//starts it one way and then sets to standby
void loop() {
  buttonInst = digitalRead(buttonPin);
  if(buttonInst == HIGH && !currentActive()){
      dispense(oat);
      delay(100);
      dispense(rasin);
      delay(100);
      dispense(sugar);
  }
  
  
}
void cook(){
  int startCookTime = millis();
  int startCycleTime = millis();
  int PWM_Value = 255;
  while(millis() - startCookTime < cookTimeOat*1000){
    analogWrite(mosfetPin, PWM_Value);
    if(millis() - startCycleTime > 2000){
      float CurentT = getTemprature();
      PWM_Value = PID(CurentT, millis() - startCycleTime);
      startCycleTime=millis();
    }
  }
}
int PID(float Temp,int Time){
  float error = IdealTemp-Temp;
  float porportion = Kp * error;
  float derivative =  Kd * (error-PreviousError)/Time;
  TotalIntergral+=Time*(PreviousError+(error-PreviousError)/2);
  float intergral =  Ki * TotalIntergral;
  PreviousError = error;
  int PWM = (int) (porportion+derivative+intergral);
  if(PWM<0){
    PWM=0;
  } else if(PWM>255){
    PWM=255;
  }
  return PWM;
}
float getTemprature(){
  float sensorValue;
  float voltageOut;
  float temperatureC; 
  sensorValue = analogRead(sensorPin);
  voltageOut = (sensorValue * 5000) / 1024;
  temperatureC = voltageOut / 10;
  Serial.print("Temperature(ºC): ");
  Serial.print(temperatureC);
  Serial.print("  Voltage(mV): ");
  Serial.println(voltageOut);
  return temperatureC;
  delay(10);
} 
void dispense(int servo){
  servoActive[servo] = true;
  servos[servo].write(180);                  
  delay(200); 
  servos[servo].write(90);
  servoActive[servo] = false;
}
bool currentActive(){
  for(int i=0;i<5;i++){
    if(servoActive[i]) return true;
  }
  return false;
}