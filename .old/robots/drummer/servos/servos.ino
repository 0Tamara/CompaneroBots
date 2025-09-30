#include <ESP32Servo.h>

#define R_ARM_PIN 13
#define L_ARM_PIN 12
#define KICK_PIN 32

#define R_UP 60
#define R_DOWN 50
#define L_UP 20
#define L_DOWN 30
#define K_UP 85
#define K_DOWN 90

Servo r_arm;  //0-80 = down-front
Servo l_arm;  //80-0 = down-front
Servo kick;   //85-90-85 = kick

void setup()
{
  Serial.begin(115200);

  r_arm.attach(R_ARM_PIN);
  l_arm.attach(L_ARM_PIN);
  kick.attach(KICK_PIN);
}

void loop()
{
  kick.write(K_DOWN);
  r_arm.write(R_DOWN);
  l_arm.write(L_DOWN);
  delay(100);
  kick.write(K_UP);
  r_arm.write(R_UP);
  l_arm.write(L_UP);
  delay(1000);
}
