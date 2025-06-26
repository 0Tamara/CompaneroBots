#include <ESP32Servo.h>

#define R_ARM_PIN   13
#define L_ARM_PIN   12
#define R_ELBOW_PIN 14
#define L_ELBOW_PIN 27

Servo r_arm, l_arm, r_elbow, l_elbow;

void servoRamp(Servo moving, byte start, byte end)
{
  if(start < end)
  {
    for(int i=start; i<=end; i++)
    {
      moving.write(i);
      delay(20);
    }
  } else
  {
    for(int i=start; i>=end; i--)
    {
      moving.write(i);
      delay(20);
    }
  }
}

void setup()
{
  Serial.begin(115200);

  r_arm.attach(R_ARM_PIN);
  l_arm.attach(L_ARM_PIN);
  r_elbow.attach(R_ELBOW_PIN);
  l_elbow.attach(L_ELBOW_PIN);

  r_arm.write(180);
  l_arm.write(0);
  r_elbow.write(70);
  l_elbow.write(0);
  delay(1000);
}

void loop()
{
  servoRamp(r_elbow, 0, 90);
}
