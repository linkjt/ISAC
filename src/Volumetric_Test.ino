#include <Servo.h>

const int buttonPin = 2; 

const int ServoSpeed = 180;
const int ServoDelay = 2000;

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
void dispense(int servo){
  servoActive[servo] = true;
  servos[servo].write(90+ServoSpeed);                  
  delay(ServoDelay); 
  servos[servo].write(90);
  servoActive[servo] = false;
}
bool currentActive(){
  for(int i=0;i<5;i++){
    if(servoActive[i]) return true;
  }
  return false;
}