#include <ESP32Servo.h>

#define R_ARM_PIN   13
#define L_ARM_PIN   12
#define R_ELBOW_PIN 14
#define L_ELBOW_PIN 27

Servo r_arm, l_arm, r_elbow, l_elbow;

void servoRamp(byte end, int min_delay, int max_delay)
{
  int t;
  byte start = r_elbow.read()+1;
  if(start < end)
  {
    for(int i=start; i<=end; i++)
    {
      //t = map(i, start, (start+end)/2, 0, max_delay);
      if(i<((start+end)/2))
        t = map(i, start, (start+end)/2, max_delay, min_delay);
      else
        t = map(i, (start+end)/2, end, min_delay, max_delay);

      r_elbow.write(i);
      delay(t);
      Serial.printf("position: %d\tdelay: %d\n", i, t);
    }
  } else
  {
    for(int i=start; i>=end; i--)
    {
      if(i>((start+end)/2))
        t = map(i, start, (start+end)/2, max_delay, min_delay);
      else
        t = map(i, (start+end)/2, end, min_delay, max_delay);

      r_elbow.write(i);
      delay(t);
      Serial.printf("position: %d\tdelay: %d\n", i, t);
    }
  }
  Serial.println();
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
  servoRamp(10, 5, 20);
  servoRamp(80, 5, 20);
}
