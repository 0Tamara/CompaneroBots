#include <ESP32Servo.h>

#define R_ARM_PIN 27
//#define L_ARM_PIN 21
//#define KICK_PIN 19

Servo r_arm;  //0-80 = down-front
//Servo l_arm;  //80-0 = down-front
//Servo kick;   //85-90-85 = kick

void setup()
{
  Serial.begin(115200);

  r_arm.attach(R_ARM_PIN);
  //l_arm.attach(L_ARM_PIN);
  //kick.attach(KICK_PIN);
}

void loop()
{
  r_arm.write(180);
}
