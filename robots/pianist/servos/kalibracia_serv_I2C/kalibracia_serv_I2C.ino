#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pca9685 = Adafruit_PWMServoDriver(0x40);
#define SERVOMIN 125
#define SERVOMAX 575

int angleToPulse(int angle) {
  return map(angle, 0, 180, SERVOMIN, SERVOMAX);
}

void setup() {
  Serial.begin(115200);
  pca9685.begin();
  pca9685.setPWMFreq(50); 
}

void loop() {
  Serial.println("Servo na 0°");
  pca9685.setPWM(0, 0, SERVOMIN); 
  delay(1000);
  Serial.println("Servo na 90°");
  pca9685.setPWM(0, 0, angleToPulse(90)); 
  delay(1000);
  Serial.println("Servo na 180°");
  pca9685.setPWM(0, 0, SERVOMAX); // pokial seka, tak zniz na kus menej
  delay(1000);
}