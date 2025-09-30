#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pca9685right(0x41, Wire);
Adafruit_PWMServoDriver pca9685left(0x40, Wire);
#define SERVOMIN 125
#define SERVOMAX 575

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  pca9685right.begin();
  pca9685right.setPWMFreq(50); 
  pca9685left.begin();
  pca9685left.setPWMFreq(50); 
}

void loop() {
  /*Serial.println("Servo na 0°");
  pca9685.setPWM(0, 0, SERVOMIN); 
  delay(1000);
  Serial.println("Servo na 90°");
  pca9685.setPWM(0, 0, angleToPulse(90)); 
  delay(1000);
  Serial.println("Servo na 180°");
  pca9685.setPWM(0, 0, SERVOMAX); // pokial seka, tak zniz na kus menej
  delay(1000);*/
  for(int i=8; i<16; i++)
  {
    pca9685left.setPWM(i, 0, SERVOMAX);
    pca9685right.setPWM(i, 0, SERVOMIN);
  }
}