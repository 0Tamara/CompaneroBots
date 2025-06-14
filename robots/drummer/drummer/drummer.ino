#include <ESP32Servo.h>
#include <Adafruit_NeoPixel.h>

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
#define LED_PIN_K 32    //kick drum
#define LED_COUNT_K 54
#define LED_PIN_R 14    //right drum
#define LED_COUNT_R 36
#define LED_PIN_EYES 23
#define LED_COUNT_EYES 50

Adafruit_NeoPixel left_ring(LED_COUNT_L, LED_PIN_L, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel kick_ring(LED_COUNT_K, LED_PIN_K, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel right_ring(LED_COUNT_R, LED_PIN_R, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel eyes(LED_COUNT_EYES, LED_PIN_EYES, NEO_GRB + NEO_KHZ800);

unsigned long timer_kick;
unsigned long timer_right;
unsigned long timer_left;
int kicks;
int snares;

byte color_eyes[] = {40, 1, 2}; //pink
byte colors_drums[6][3] = {{10, 0, 0},  //colors that will be cycling over
                           {5, 5, 0},
                           {0, 10, 0},
                           {0, 5, 5},
                           {0, 0, 10},
                           {5, 0, 5}};
int color_index_kick = 0;
int color_index_left = 0;
int color_index_right = 0;

TaskHandle_t Task1;

// Zapína LED na všetkých pásikoch naraz
void ledky_vedlajsie_left() {
  for (int i = 0; i < LED_COUNT_R; i++) {
    left_ring.setPixelColor(i, 10, 0, 0);

    left_ring.show();
    delay(50);
  }

  for (int i = LED_COUNT_R - 1; i >= 0; i--) {
    left_ring.setPixelColor(i, 0, 0, 0);

    left_ring.show();
    delay(50);
  }
}
void ledky_vedlajsie_right() {
  for (int i = 0; i < LED_COUNT_R; i++) {
    right_ring.setPixelColor(i, 10, 0, 0);

    right_ring.show();
    delay(50);
  }

  for (int i = LED_COUNT_R - 1; i >= 0; i--) {
    right_ring.setPixelColor(i, 0, 0, 0);

    right_ring.show();
    delay(50);
  }
}
void kick_ring_bubon() {
  for (int i = 0; i < LED_COUNT_K; i++) {
    kick_ring.setPixelColor(i, 10, 0, 0);
    kick_ring.show();
    delay(50);
  }
  for (int i = LED_COUNT_K - 1; i >= 0; i--) {
    kick_ring.setPixelColor(i, 0, 0, 0);
    kick_ring.show();
    delay(50);
  }
}

void changeColorsKick()
{
  if(color_index_kick < 5)
    color_index_kick++;
  else
    color_index_kick = 0;
  for(int i=0; i<LED_COUNT_K; i++)
    kick_ring.setPixelColor(i, colors_drums[color_index_kick][0], colors_drums[color_index_kick][1], colors_drums[color_index_kick][2]);
  kick_ring.show();
}
void changeColorsLeft()
{
  if(color_index_left < 5)
    color_index_left++;
  else
    color_index_left = 0;
  for(int i=0; i<LED_COUNT_L; i++)
    left_ring.setPixelColor(i, colors_drums[color_index_left][0], colors_drums[color_index_left][1], colors_drums[color_index_left][2]);
  left_ring.show();
}
void changeColorsRight()
{
  if(color_index_right < 5)
    color_index_right++;
  else
    color_index_right = 0;
  for(int i=0; i<LED_COUNT_R; i++)
    right_ring.setPixelColor(i, colors_drums[0][color_index_right], colors_drums[1][color_index_right], colors_drums[2][color_index_right]);
  right_ring.show();
}
/*
//---music functions---
void freedom()
{
  timer_kick = 0;
  timer_right = 0;
  kicks = 0;
  snares = 0;
  while(kicks < 4)
  {
    if((millis()-timer_kick) >= 570)
    {
      timer_kick = millis();
      kick.write(90);
      changeColorsKick();
      delay(100);
      kick.write(85);
      kicks ++;
    }
    if(snares < 3)
    {
      if((millis()-timer_right) >= 430)
      {
        timer_right = millis();
        r_arm.write(R_DOWN);
        l_arm.write(L_DOWN);
        changeColorsLeft();
        changeColorsRight();
        delay(100);
        r_arm.write(R_UP);
        l_arm.write(L_UP);
        snares ++;
      }
    }
    else
    {
      if(kicks == 3)
      {
        delay(300);
        r_arm.write(R_DOWN);
        changeColorsRight();
        delay(100);
        r_arm.write(R_UP);
        l_arm.write(L_DOWN);
        changeColorsLeft();
        delay(100);
        l_arm.write(L_UP);
        r_arm.write(R_DOWN);
        changeColorsRight();
        delay(100);
        r_arm.write(R_UP);
      }
    }
  }
}

void fireball_clapping()
{
  timer_right = 0;
  snares = 0;
  while(snares < 4)
  {
    if(!(snares%2))
    {
      if((millis()-timer_right) >= 480)
      {
        timer_right = millis();
        r_arm.write(R_DOWN);
        changeColorsRight();
        delay(100);
        r_arm.write(R_UP);
        snares ++;
      }
    }
    else
    {
      if((millis()-timer_right) >= 480)
      {
        timer_right = millis();
        l_arm.write(L_DOWN);
        changeColorsLeft();
        delay(100);
        l_arm.write(L_UP);
        snares ++;
      }
    }
  }
}

void fireball_drop()
{
  timer_kick = 0;
  kicks = 0;
  while(kicks < 4)
  {
    if((millis()-timer_kick) >= 480)
    {
      timer_kick = millis();
      if(!(kicks%2))
      {
        kick.write(90);
        r_arm.write(R_DOWN);
        l_arm.write(L_DOWN);
        changeColorsKick();
        changeColorsRight();
        changeColorsLeft();
        delay(100);
        kick.write(85);
        r_arm.write(R_UP);
        l_arm.write(L_UP);
      }
      else
      {
        if(kicks == 1)
        {
          kick.write(90);
          r_arm.write(R_DOWN);
          changeColorsKick();
          changeColorsRight();
          delay(100);
          kick.write(85);
          r_arm.write(R_UP);
          while((millis()-timer_kick) < 240) delay(10);
          l_arm.write(L_DOWN);
          changeColorsLeft();
          delay(100);
          l_arm.write(L_UP);
        }
        else
        {
          kick.write(90);
          l_arm.write(L_DOWN);
          changeColorsKick();
          changeColorsLeft();
          delay(100);
          kick.write(85);
          l_arm.write(L_UP);
          while((millis()-timer_kick) < 240) delay(10);
          r_arm.write(R_DOWN);
          changeColorsRight();
          delay(100);
          r_arm.write(R_UP);
        }
      }
      kicks ++;
    }
  }
}

void fireball_bass()
{
  timer_kick = 0;
  kicks = 0;
  while(kicks < 4)
  {
    if((millis()-timer_kick) >= 480)
    {
      timer_kick = millis();
      kick.write(90);
      changeColorsKick();
      delay(100);
      kick.write(85);
      kicks ++;
    }
  }
}


void fireball_chill()
{
  timer_kick = millis();
  kick.write(90);
  r_arm.write(R_DOWN);
  l_arm.write(L_DOWN);
  changeColorsKick();
  changeColorsRight();
  changeColorsLeft();
  delay(100);
  kick.write(85);
  r_arm.write(R_UP);
  l_arm.write(L_UP);
  snares = 0;
  while(snares < 3)
  {
    if(snares%2)
    {
      if((millis()-timer_right) >= 480)
      {
        timer_right = millis();
        r_arm.write(R_DOWN);
        changeColorsRight();
        delay(100);
        r_arm.write(R_UP);
        snares ++;
      }
    }
    else
    {
      if((millis()-timer_right) >= 480)
      {
        timer_right = millis();
        l_arm.write(L_DOWN);
        changeColorsLeft();
        delay(100);
        l_arm.write(L_UP);
        snares ++;
      }
    }
  }
}
*/
//---LEDs functions---
void closeEyes()  //cca 300ms
{
  // Blink LEDs in reverse order (off in sections)
  for (int i = 20; i < 25; i++) eyes.setPixelColor(i, 0, 0, 0); // Left eye
  for (int i = 45; i < 50; i++) eyes.setPixelColor(i, 0, 0, 0); // Right eye
  eyes.show();

  delay(50);

  // Now let's go down the LED sections
  for (int i = 15; i < 20; i++) eyes.setPixelColor(i, 0, 0, 0);
  for (int i = 40; i < 45; i++) eyes.setPixelColor(i, 0, 0, 0);
  eyes.show();

  delay(50);

  for (int i = 10; i < 15; i++) eyes.setPixelColor(i, 0, 0, 0);
  for (int i = 35; i < 40; i++) eyes.setPixelColor(i, 0, 0, 0);
  eyes.show();

  delay(50);

  for (int i = 5; i < 10; i++) eyes.setPixelColor(i, 0, 0, 0);
  for (int i = 30; i < 35; i++) eyes.setPixelColor(i, 0, 0, 0);
  eyes.show();

  delay(50);

  for (int i = 0; i < 5; i++) eyes.setPixelColor(i, 0, 0, 0);
  for (int i = 25; i < 30; i++) eyes.setPixelColor(i, 0, 0, 0);
  eyes.show();

  delay(50);
}

void openEyes(uint8_t red, uint8_t green, uint8_t blue)  //cca 300ms
{
  // Blink LEDs in reverse order (turning LEDs back on)
  for (int i = 0; i < 5; i++) eyes.setPixelColor(i, red, green, blue); // Left eye
  for (int i = 25; i < 30; i++) eyes.setPixelColor(i, red, green, blue); // Right eye
  eyes.show();

  delay(50);

  for (int i = 5; i < 10; i++) eyes.setPixelColor(i, red, green, blue);
  for (int i = 30; i < 35; i++) eyes.setPixelColor(i, red, green, blue);
  eyes.show();

  delay(50);

  for (int i = 10; i < 15; i++) eyes.setPixelColor(i, red, green, blue);
  for (int i = 35; i < 40; i++) eyes.setPixelColor(i, red, green, blue);
  eyes.show();

  delay(50);

  for (int i = 15; i < 20; i++) eyes.setPixelColor(i, red, green, blue);
  for (int i = 40; i < 45; i++) eyes.setPixelColor(i, red, green, blue);
  eyes.show();

  delay(50);

  for (int i = 20; i < 25; i++) eyes.setPixelColor(i, red, green, blue);
  for (int i = 45; i < 50; i++) eyes.setPixelColor(i, red, green, blue);
  eyes.show();

  delay(50);
}

void loop_2(void * pvParameters)
{
  while(1)
  {
    ledky_vedlajsie_left();
    kick_ring_bubon();
  }
}

//---main code---
void setup()
{
  Serial.begin(115200);
  Serial.println("I'm on");

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

  left_ring.begin();
  Serial.println();
  kick_ring.begin();
  right_ring.begin();
  eyes.begin();
  eyes.show();
  Serial.println("I'm set");

  for (int i = 0; i < 54; i++) {
    if (i < LED_COUNT_R) right_ring.setPixelColor(i, 10, 10, 10);
    if (i < LED_COUNT_L) left_ring.setPixelColor(i, 10, 10, 10);
    if (i < LED_COUNT_K) kick_ring.setPixelColor(i, 10, 10, 10);
  }

  left_ring.show();
  kick_ring.show();
  right_ring.show();
  Serial.println("I'm white");
  delay(1000);
  
}

void loop()
{ 
  ledky_vedlajsie_right();
  //closeEyes();
  //openEyes(32, 32, 32);
}
