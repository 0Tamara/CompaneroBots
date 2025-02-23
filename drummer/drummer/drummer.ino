#include <ESP32Servo.h>

#define R_ARM_PIN 18
#define L_ARM_PIN 21
#define KICK_PIN 19

Servo r_arm;  //0-80 = down-front
Servo l_arm;  //80-0 = down-front
Servo kick;   //85-90-85 = kick

unsigned long timer_kick;
unsigned long timer_right;
unsigned long timer_left;
int kicks;
int snares;

void freedom()
{
  timer_kick = 0;
  timer_right = 0;
  kicks = 0;
  snares = 0;
  while(kicks < 4)
  {
    if((millis()-timer_kick) >= (570-25))
    {
      kick.write(90);
      delay(50);
      kick.write(85);
      timer_kick = millis();
      kicks ++;
    }
    if(snares < 3)
    {
      if((millis()-timer_right) >= (430-25))
      {
        r_arm.write(70);
        l_arm.write(10);
        delay(100);
        r_arm.write(80);
        l_arm.write(0);
        timer_right = millis();
        snares ++;
      }
    }
    else
    {
      if(kicks == 3)
      {
        delay(300);
        r_arm.write(70);
        delay(100);
        r_arm.write(80);
        l_arm.write(10);
        delay(100);
        l_arm.write(0);
        r_arm.write(70);
        delay(100);
        r_arm.write(80);
      }
    }
  }
}

void setup()
{
  Serial.begin(115200);

  r_arm.attach(R_ARM_PIN);
  l_arm.attach(L_ARM_PIN);
  kick.attach(KICK_PIN);
  
  kick.write(85);
  r_arm.write(80);
  l_arm.write(0);
}

void loop()
{
  Serial.print(millis());
  Serial.println(" start");
  freedom();
  Serial.print(millis());
  Serial.println(" end");
  delay(570-25);
}
