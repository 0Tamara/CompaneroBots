#include <ESP32Servo.h>
#include <FastLED.h>

#define R_ARM_PIN 13
#define L_ARM_PIN 12
#define KICK_PIN 33

#define R_UP 80
#define R_DOWN 70
#define L_UP 0
#define L_DOWN 10

Servo r_arm;  //0-80 = down-front
Servo l_arm;  //80-0 = down-front
Servo kick;   //85-90-85 = kick

#define LED_PIN_L 27     //left drum
#define LED_COUNT_L 36
CRGB left_ring[LED_COUNT_L];
#define LED_PIN_K 32    //kick drum
#define LED_COUNT_K 54
CRGB kick_ring[LED_COUNT_K];
#define LED_PIN_R 14    //right drum
#define LED_COUNT_R 36
CRGB right_ring[LED_COUNT_R];
#define LED_PIN_EYES 23
#define LED_COUNT_EYES 50
CRGB eyes[LED_COUNT_EYES];

unsigned long timer_kick;
unsigned long timer_right;
unsigned long timer_left;
int kicks;
int snares;

uint color_eyes = 0x200102; //pink
uint colors_drums[6] = {0xFF0000,  //colors that will be cycling over
                        0x808000,
                        0x00FF00,
                        0x008080,
                        0x0000FF,
                        0x800080};

TaskHandle_t Task1;
int rising_color[3] = {0, 0, 0};  //color used for rising effects (during music)
bool blink_drums[3] = {0, 0, 0};  //rising effects active
/*
// Zapína LED na všetkých pásikoch naraz
void ledky_vedlajsie() {
  for (int i = 0; i < LED_COUNT_R; i++) {
    left_ring[i] = 0xFF0000;
    right_ring[i] = 0xFF0000;

    FastLED.show();
    delay(50);
  }

  for (int i = LED_COUNT_R - 1; i >= 0; i--) {
    left_ring[i] = 0x000000;
    right_ring[i] = 0x000000;

    FastLED.show();
    delay(50);
  }
}
void kick_ring_bubon() {
  for (int i = 0; i < LED_COUNT_K; i++) {
    kick_ring[i] = 0xFF0000;
    FastLED.show();
    delay(50);
  }
  for (int i = LED_COUNT_K - 1; i >= 0; i--) {
    kick_ring[i] = 0x000000;
    FastLED.show();
    delay(50);
  }
}

//---eyes functions---
void closeEyes()  //cca 300ms
{
  // Blink LEDs in reverse order (off in sections)
  for (int i = 20; i < 25; i++) eyes[i] = 0x000000; // Left eye
  for (int i = 45; i < 50; i++) eyes[i] = 0x000000; // Right eye
  FastLED.show();

  delay(50);

  // Now let's go down the LED sections
  for (int i = 15; i < 20; i++) eyes[i] = 0x000000;
  for (int i = 40; i < 45; i++) eyes[i] = 0x000000;
  FastLED.show();

  delay(50);

  for (int i = 10; i < 15; i++) eyes[i] = 0x000000;
  for (int i = 35; i < 40; i++) eyes[i] = 0x000000;
  FastLED.show();

  delay(50);

  for (int i = 5; i < 10; i++) eyes[i] = 0x000000;
  for (int i = 30; i < 35; i++) eyes[i] = 0x000000;
  FastLED.show();

  delay(50);

  for (int i = 0; i < 5; i++) eyes[i] = 0x000000;
  for (int i = 25; i < 30; i++) eyes[i] = 0x000000;
  FastLED.show();

  delay(50);
}

void openEyes(uint color)  //cca 300ms
{
  // Blink LEDs in reverse order (turning LEDs back on)
  for (int i = 0; i < 5; i++) eyes[i] = color; // Left eye
  for (int i = 25; i < 30; i++) eyes[i] = color; // Right eye
  FastLED.show();

  delay(50);

  for (int i = 5; i < 10; i++) eyes[i] = color;
  for (int i = 30; i < 35; i++) eyes[i] = color;
  FastLED.show();

  delay(50);

  for (int i = 10; i < 15; i++) eyes[i] = color;
  for (int i = 35; i < 40; i++) eyes[i] = color;
  FastLED.show();

  delay(50);

  for (int i = 15; i < 20; i++) eyes[i] = color;
  for (int i = 40; i < 45; i++) eyes[i] = color;
  FastLED.show();

  delay(50);

  for (int i = 20; i < 25; i++) eyes[i] = color;
  for (int i = 45; i < 50; i++) eyes[i] = color;
  FastLED.show();

  delay(50);
}*/

void loop_2(void* parameter)  //loop for rising effects (during music)
{
  //---setup2---
  int LEDs_pos[3] = {0, 0, LED_COUNT_R-1};  //position on the LED ring
  bool rising[3] = {1, 1, 1};  //rising / lowering
  bool miss_out = 0;  //missing out every other step to go slower
  //---loop2---
  while(true)
  {
    if(blink_drums[0])  //left effect active
    {
      if(rising[0])  //lighting up
      {
        left_ring[LEDs_pos[0]] = rising_color[0];
        LEDs_pos[0] ++;

        if(LEDs_pos[0] == LED_COUNT_L)  //prepare values for turn off stage
        {
          rising[0] = 0;
          LEDs_pos[0] = LED_COUNT_L-1;
        }
      }
      else  //turning off
      {
        left_ring[LEDs_pos[0]] = 0;
        LEDs_pos[0] --;

        if(LEDs_pos[0] < 0)  //revert values
        {
          rising[0] = 1;
          blink_drums[0] = 0;
          LEDs_pos[0] = 0;
        }
      }
    }

    if(blink_drums[1])  //kick effect active
    {
      if(rising[1])  //lighting up
      {
        kick_ring[LEDs_pos[1]] = rising_color[1];
        kick_ring[(LED_COUNT_K-1)-LEDs_pos[1]] = rising_color[1];  //going from both sides to middle
        LEDs_pos[1] ++;

        if(LEDs_pos[1] == LED_COUNT_K/2)  //prepare values for turn off stage
        {
          rising[1] = 0;
          LEDs_pos[1] = LED_COUNT_K/2-1;
        }
      }
      else  //turning off
      {
        kick_ring[LEDs_pos[1]] = 0;
        kick_ring[(LED_COUNT_K-1)-LEDs_pos[1]] = 0;
        if(!miss_out)  //turning off half the speed
          LEDs_pos[1] --;
        miss_out = !miss_out;

        if(LEDs_pos[1] < 0)  //revert values
        {
          rising[1] = 1;
          blink_drums[1] = 0;
          LEDs_pos[1] = 0;
        }
      }
    }

    if(blink_drums[2])  //right effect active
    {
      if(rising[2])
      {
        right_ring[LEDs_pos[2]] = rising_color[2];
        LEDs_pos[2] --;

        if(LEDs_pos[2] < 0)  //prepare values for turn off stage
        {
          rising[2] = 0;
          LEDs_pos[2] = 0;
        }
      }
      else
      {
        right_ring[LEDs_pos[2]] = 0;
        LEDs_pos[2] ++;

        if(LEDs_pos[2] == LED_COUNT_R)  //revert values
        {
          rising[2] = 1;
          blink_drums[2] = 0;
          LEDs_pos[2] = LED_COUNT_R-1;
        }
      }
    }
    
    FastLED.show();
    delay(20);
  }
}

//---main code---
void setup()
{
  Serial.begin(115200);

  r_arm.attach(R_ARM_PIN);
  l_arm.attach(L_ARM_PIN);
  kick.attach(KICK_PIN);
  
  kick.write(85);
  r_arm.write(R_UP);
  l_arm.write(L_UP);

  xTaskCreatePinnedToCore(
      loop_2, /* Function to implement the task */
      "Task1", /* Name of the task */
      10000,  /* Stack size in words */
      NULL,  /* Task input parameter */
      0,  /* Priority of the task */
      &Task1,  /* Task handle. */
      0); /* Core where the task should run */

  FastLED.addLeds<WS2811, LED_PIN_L, GRB>(left_ring, LED_COUNT_L);
  FastLED.addLeds<WS2811, LED_PIN_K, GRB>(kick_ring, LED_COUNT_K);
  FastLED.addLeds<WS2811, LED_PIN_R, GRB>(right_ring, LED_COUNT_R);
  FastLED.addLeds<WS2811, LED_PIN_EYES, GRB>(eyes, LED_COUNT_EYES);

  FastLED.setBrightness(32);

  for (int i = 0; i < 54; i++) {
    if (i < LED_COUNT_R) right_ring[i] = 0x808080;
    if (i < LED_COUNT_L) left_ring[i] = 0x808080;
    if (i < LED_COUNT_K) kick_ring[i] = 0x808080;
  }

  FastLED.show();
  delay(1000);
  for (int i = 0; i < 54; i++) {
    if (i < LED_COUNT_R) right_ring[i] = 0x000000;
    if (i < LED_COUNT_L) left_ring[i] = 0x000000;
    if (i < LED_COUNT_K) kick_ring[i] = 0x000000;
  }
  FastLED.show();
}

void loop()
{
  for(int i=0; i<6; i++)
  {
    rising_color[0] = colors_drums[i];
    blink_drums[0] = 1;
    rising_color[1] = colors_drums[i];
    blink_drums[1] = 1;
    rising_color[2] = colors_drums[i];
    blink_drums[2] = 1;
    delay(3000);
  }
}
