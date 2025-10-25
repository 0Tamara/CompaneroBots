#include <ESP32Servo.h>
#include <FastLED.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

//-pins-
#define L_SERVO_PIN 12
#define R_SERVO_PIN 13
#define KICK_SERVO_PIN 32
#define LED_PIN_L 27
#define LED_PIN_R 14
#define LED_PIN_K 33
#define LED_PIN_EYES 23
//-values-
#define L_SERVO_RELEASE 20
#define L_SERVO_HIT 30
#define R_SERVO_RELEASE 60
#define R_SERVO_HIT 50
#define KICK_SERVO_RELEASE 85
#define KICK_SERVO_HIT 90
#define SIDES_LED_COUNT 36  //count of leds on both side drums
#define KICK_LED_COUNT 54
#define EYES_LED_COUNT 50
//-drivers-
Servo left_servo;  //80-0 = down-front
Servo right_servo;  //0-80 = down-front
Servo kick_servo;   //85-90-85 = kick
CRGB left_LEDs[SIDES_LED_COUNT];
CRGB right_LEDs[SIDES_LED_COUNT];
CRGB kick_LEDs[KICK_LED_COUNT];
CRGB eyes_LEDs[EYES_LED_COUNT];

unsigned long timer_kick;
unsigned long timer_right;
unsigned long timer_left;
unsigned long timer_music;
int num_kicks;
int num_snares;

uint color_eyes = 0x800080;
uint color_palette[6] = {0x800000,  //colors cycling over
                        0x404000,
                        0x008000,
                        0x004040,
                        0x000080,
                        0x400040};
int color_index_kick = 0;
int color_index_left = 0;
int color_index_right = 0;

TaskHandle_t Task1;
int rising_color[3] = {0, 0, 0};  //color used for rising effects (during music)
bool blink_drums[3] = {0, 0, 0};  //rising effects active
int LEDs_pos[3] = {0, 0, SIDES_LED_COUNT-1};  //position on the LED ring
bool rising[3] = {1, 1, 1};  //rising / lowering
bool miss_out[3] = {0, 0, 0};  //missing out every other step to go slower

int current_song = 0;

uint8_t pianist_addr[] = {0x84, 0x0D, 0x8E, 0xE4, 0xB4, 0x58};  //pianist MAC addr
uint8_t cam_addr[] = {0xC0, 0x49, 0xEF, 0xD0, 0x8C, 0xC0};  //camera esp MAC addr
esp_now_peer_info_t peer_info;

typedef struct struct_mes
{
  byte song;
  byte sync;
} struct_mes;

struct_mes pianist_mes;
struct_mes recv_data;

void sideLEDsRise(uint color) {
  for (int i = 0; i < SIDES_LED_COUNT; i++) {
    left_LEDs[i] = color;
    right_LEDs[i] = color;

    FastLED.show();
    delay(50);
  }

  for (int i = SIDES_LED_COUNT - 1; i >= 0; i--) {
    left_LEDs[i] = 0x000000;
    right_LEDs[i] = 0x000000;

    FastLED.show();
    delay(50);
  }
}
void kickLEDsRise(uint color) {
  for (int i = 0; i < KICK_LED_COUNT/2; i++) {
    kick_LEDs[i] = color;
    kick_LEDs[(KICK_LED_COUNT-1)-i] = color;
    FastLED.show();
    delay(50);
  }
  for (int i = KICK_LED_COUNT - 1; i >= 0; i--) {
    kick_LEDs[i] = 0x000000;
    kick_LEDs[(KICK_LED_COUNT-1)-i] = 0x000000;
    FastLED.show();
    delay(50);
  }
}

//---change colors when the drum gets hit---
void changeColorsLeft()
{
  if(color_index_left < 5)
    color_index_left++;
  else
    color_index_left = 0;
  rising_color[0] = color_palette[color_index_left];
  rising[0] = 1;
  blink_drums[0] = 1;
}
void changeColorsKick()
{
  if(color_index_kick < 5)
    color_index_kick++;
  else
    color_index_kick = 0;
  rising_color[1] = color_palette[color_index_kick];
  rising[1] = 1;
  blink_drums[1] = 1;
}
void changeColorsRight()
{
  if(color_index_right < 5)
    color_index_right++;
  else
    color_index_right = 0;
  rising_color[2] = color_palette[color_index_right];
  rising[2] = 1;
  blink_drums[2] = 1;
}

//---music functions---
void freedom()
{
  timer_kick = 0;
  timer_right = 0;
  num_kicks = 0;
  num_snares = 0;
  while(num_kicks < 4)
  {
    if((millis()-timer_kick) >= 570)
    {
      timer_kick = millis();
      kick_servo.write(KICK_SERVO_HIT);
      changeColorsKick();
      delay(100);
      kick_servo.write(KICK_SERVO_RELEASE);
      num_kicks ++;
    }
    if(num_snares < 3)
    {
      if((millis()-timer_right) >= 430)
      {
        timer_right = millis();
        right_servo.write(R_SERVO_HIT);
        left_servo.write(L_SERVO_HIT);
        changeColorsLeft();
        changeColorsRight();
        delay(100);
        right_servo.write(R_SERVO_RELEASE);
        left_servo.write(L_SERVO_RELEASE);
        num_snares ++;
      }
    }
    else
    {
      if(num_kicks == 3)
      {
        delay(300);
        right_servo.write(R_SERVO_HIT);
        changeColorsRight();
        delay(100);
        right_servo.write(R_SERVO_RELEASE);
        left_servo.write(L_SERVO_HIT);
        changeColorsLeft();
        delay(100);
        left_servo.write(L_SERVO_RELEASE);
        right_servo.write(R_SERVO_HIT);
        changeColorsRight();
        delay(100);
        right_servo.write(R_SERVO_RELEASE);
      }
    }
  }
}

void fireball_clapping()
{
  timer_right = 0;
  num_snares = 0;
  while(num_snares < 4)
  {
    if(!(num_snares%2))
    {
      if((millis()-timer_right) >= 480)
      {
        timer_right = millis();
        right_servo.write(R_SERVO_HIT);
        changeColorsRight();
        delay(100);
        right_servo.write(R_SERVO_RELEASE);
        num_snares ++;
      }
    }
    else
    {
      if((millis()-timer_right) >= 480)
      {
        timer_right = millis();
        left_servo.write(L_SERVO_HIT);
        changeColorsLeft();
        delay(100);
        left_servo.write(L_SERVO_RELEASE);
        num_snares ++;
      }
    }
  }
}

void fireball_drop()
{
  timer_kick = 0;
  num_kicks = 0;
  while(num_kicks < 4)
  {
    if((millis()-timer_kick) >= 480)
    {
      timer_kick = millis();
      if(!(num_kicks%2))
      {
        kick_servo.write(KICK_SERVO_HIT);
        right_servo.write(R_SERVO_HIT);
        left_servo.write(L_SERVO_HIT);
        changeColorsKick();
        changeColorsRight();
        changeColorsLeft();
        delay(100);
        kick_servo.write(KICK_SERVO_RELEASE);
        right_servo.write(R_SERVO_RELEASE);
        left_servo.write(L_SERVO_RELEASE);
      }
      else
      {
        if(num_kicks == 1)
        {
          kick_servo.write(KICK_SERVO_HIT);
          right_servo.write(R_SERVO_HIT);
          changeColorsKick();
          changeColorsRight();
          delay(100);
          kick_servo.write(KICK_SERVO_RELEASE);
          right_servo.write(R_SERVO_RELEASE);
          while((millis()-timer_kick) < 240) delay(10);
          left_servo.write(L_SERVO_HIT);
          changeColorsLeft();
          delay(100);
          left_servo.write(L_SERVO_RELEASE);
        }
        else
        {
          kick_servo.write(KICK_SERVO_HIT);
          left_servo.write(L_SERVO_HIT);
          changeColorsKick();
          changeColorsLeft();
          delay(100);
          kick_servo.write(KICK_SERVO_RELEASE);
          left_servo.write(L_SERVO_RELEASE);
          while((millis()-timer_kick) < 240) delay(10);
          right_servo.write(R_SERVO_HIT);
          changeColorsRight();
          delay(100);
          right_servo.write(R_SERVO_RELEASE);
        }
      }
      num_kicks ++;
    }
  }
}

void fireball_bass()
{
  timer_kick = 0;
  num_kicks = 0;
  while(num_kicks < 4)
  {
    if((millis()-timer_kick) >= 480)
    {
      timer_kick = millis();
      kick_servo.write(KICK_SERVO_HIT);
      changeColorsKick();
      delay(100);
      kick_servo.write(KICK_SERVO_RELEASE);
      num_kicks ++;
    }
  }
}


void fireball_chill()
{
  timer_kick = millis();
  kick_servo.write(KICK_SERVO_HIT);
  right_servo.write(R_SERVO_HIT);
  left_servo.write(L_SERVO_HIT);
  changeColorsKick();
  changeColorsRight();
  changeColorsLeft();
  delay(100);
  kick_servo.write(KICK_SERVO_RELEASE);
  right_servo.write(R_SERVO_RELEASE);
  left_servo.write(L_SERVO_RELEASE);
  num_snares = 0;
  while(num_snares < 3)
  {
    if(num_snares%2)
    {
      if((millis()-timer_right) >= 480)
      {
        timer_right = millis();
        right_servo.write(R_SERVO_HIT);
        changeColorsRight();
        delay(100);
        right_servo.write(R_SERVO_RELEASE);
        num_snares ++;
      }
    }
    else
    {
      if((millis()-timer_right) >= 480)
      {
        timer_right = millis();
        left_servo.write(L_SERVO_HIT);
        changeColorsLeft();
        delay(100);
        left_servo.write(L_SERVO_RELEASE);
        num_snares ++;
      }
    }
  }
}

//---eyes functions---
void closeEyes()  //~300ms
{
  for(int i=0; i<5; i++)
  {
    for (int j = 20; j < 25; j++) eyes_LEDs[j] = 0x000000; // Left eye
    for (int j = 45; j < 50; j++) eyes_LEDs[j] = 0x000000; // Right eye
    FastLED.show();
    delay(50);
  }
  // Blink LEDs in reverse order (off in sections)
  for (int i = 20; i < 25; i++) eyes_LEDs[i] = 0x000000; // Left eye
  for (int i = 45; i < 50; i++) eyes_LEDs[i] = 0x000000; // Right eye
  FastLED.show();

  delay(50);

  // Now let's go down the LED sections
  for (int i = 15; i < 20; i++) eyes_LEDs[i] = 0x000000;
  for (int i = 40; i < 45; i++) eyes_LEDs[i] = 0x000000;
  FastLED.show();

  delay(50);

  for (int i = 10; i < 15; i++) eyes_LEDs[i] = 0x000000;
  for (int i = 35; i < 40; i++) eyes_LEDs[i] = 0x000000;
  FastLED.show();

  delay(50);

  for (int i = 5; i < 10; i++) eyes_LEDs[i] = 0x000000;
  for (int i = 30; i < 35; i++) eyes_LEDs[i] = 0x000000;
  FastLED.show();

  delay(50);

  for (int i = 0; i < 5; i++) eyes_LEDs[i] = 0x000000;
  for (int i = 25; i < 30; i++) eyes_LEDs[i] = 0x000000;
  FastLED.show();

  delay(50);
}

void openEyes(uint color)  //cca 300ms
{
  // Blink LEDs in reverse order (turning LEDs back on)
  for (int i = 0; i < 5; i++) eyes_LEDs[i] = color; // Left eye
  for (int i = 25; i < 30; i++) eyes_LEDs[i] = color; // Right eye
  FastLED.show();

  delay(50);

  for (int i = 5; i < 10; i++) eyes_LEDs[i] = color;
  for (int i = 30; i < 35; i++) eyes_LEDs[i] = color;
  FastLED.show();

  delay(50);

  for (int i = 10; i < 15; i++) eyes_LEDs[i] = color;
  for (int i = 35; i < 40; i++) eyes_LEDs[i] = color;
  FastLED.show();

  delay(50);

  for (int i = 15; i < 20; i++) eyes_LEDs[i] = color;
  for (int i = 40; i < 45; i++) eyes_LEDs[i] = color;
  FastLED.show();

  delay(50);

  for (int i = 20; i < 25; i++) eyes_LEDs[i] = color;
  for (int i = 45; i < 50; i++) eyes_LEDs[i] = color;
  FastLED.show();

  delay(50);
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incoming_data, int len) {
  memcpy(&recv_data, incoming_data, sizeof(recv_data));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("song: ");
  Serial.println(recv_data.song);
  Serial.print("sync: ");
  Serial.println(recv_data.sync);
  Serial.println();

  if(recv_data.song == 1)
  {
    openEyes(color_eyes);
  }
  if(recv_data.song == 2)
  {
    sideLEDsRise(0xFF0000);
    cam_mes.feedback = 1;
    esp_now_send(cam_addr, (uint8_t *) &cam_mes, sizeof(cam_mes));
  }
  if(recv_data.song == 3)
  {
    kickLEDsRise(0x00FF00);
    cam_mes.feedback = 1;
    esp_now_send(cam_addr, (uint8_t *) &cam_mes, sizeof(cam_mes));
  }
  if(recv_data.song == 4)
  {
    current_song = 1;
    pianist_mes.song = 4;
    pianist_mes.sync = 1;
    for (int i = 0; i < SIDES_LED_COUNT; i++)
      right_LEDs[i] = 0xFF0000;
    FastLED.show();
  }
}

//---loop for rising effects (during music)---
void loop_2(void* parameter)
{
  //---loop2---
  while(true)
  {
    if(blink_drums[0])  //left effect active
    {
      if(rising[0])  //lighting up
      {
        left_LEDs[LEDs_pos[0]] = rising_color[0];
        LEDs_pos[0] ++;

        if(LEDs_pos[0] == SIDES_LED_COUNT)  //prepare values for turn off stage
        {
          rising[0] = 0;
          LEDs_pos[0] = SIDES_LED_COUNT-1;
        }
      }
      else  //turning off
      {
        left_LEDs[LEDs_pos[0]] = 0;
        LEDs_pos[0] --;

        if(LEDs_pos[0] < 0)  //revert values
        {
          rising[0] = 1;
          blink_drums[0] = 0;
          LEDs_pos[0] = 0;
          miss_out[0] = 0;
        }
      }
    }

    if(blink_drums[1])  //kick effect active
    {
      if(rising[1])  //lighting up
      {
        kick_LEDs[LEDs_pos[1]] = rising_color[1];
        kick_LEDs[LEDs_pos[1]+1] = rising_color[1];
        kick_LEDs[(KICK_LED_COUNT-1)-LEDs_pos[1]] = rising_color[1];  //going from both sides to middle
        kick_LEDs[(KICK_LED_COUNT-1)-LEDs_pos[1]-1] = rising_color[1];
        LEDs_pos[1] +=2;

        if(LEDs_pos[1] > KICK_LED_COUNT/2-1)  //prepare values for turn off stage
        {
          rising[1] = 0;
          LEDs_pos[1] = KICK_LED_COUNT/2-1;
        }
      }
      else  //turning off
      {
        kick_LEDs[LEDs_pos[1]] = 0;
        kick_LEDs[(KICK_LED_COUNT-1)-LEDs_pos[1]] = 0;
        LEDs_pos[1] --;

        if(LEDs_pos[1] < 0)  //revert values
        {
          rising[1] = 1;
          blink_drums[1] = 0;
          LEDs_pos[1] = 0;
          miss_out[1] = 0;
        }
      }
    }

    if(blink_drums[2])  //right effect active
    {
      if(rising[2])
      {
        right_LEDs[LEDs_pos[2]] = rising_color[2];
        LEDs_pos[2] --;

        if(LEDs_pos[2] < 0)  //prepare values for turn off stage
        {
          rising[2] = 0;
          LEDs_pos[2] = 0;
        }
      }
      else
      {
        right_LEDs[LEDs_pos[2]] = 0;
        LEDs_pos[2] ++;

        if(LEDs_pos[2] == SIDES_LED_COUNT)  //revert values
        {
          rising[2] = 1;
          blink_drums[2] = 0;
          LEDs_pos[2] = SIDES_LED_COUNT-1;
          miss_out[2] = 0;
        }
      }
    }
    
    FastLED.show();
    delay(10);
  }
}

void setup()
{
  Serial.begin(115200);

  //-init WiFi & read MAC address-
  WiFi.mode(WIFI_STA);
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  Serial.printf("My MAC address: {0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X}\n",
                baseMac[0], baseMac[1], baseMac[2],
                baseMac[3], baseMac[4], baseMac[5]);
  //-init esp-now-
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  //-register peers-
  peer_info.channel = 0;  
  peer_info.encrypt = false;
  //-add peers-
  memcpy(peer_info.peer_addr, pianist_addr, 6);
  if (esp_now_add_peer(&peer_info) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }
  memcpy(peer_info.peer_addr, cam_addr, 6);
  if (esp_now_add_peer(&peer_info) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }
  //-register recieve callback-
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

  //-init servos-
  right_servo.attach(R_SERVO_PIN);
  left_servo.attach(L_SERVO_PIN);
  kick_servo.attach(KICK_SERVO_PIN);
  
  kick_servo.write(KICK_SERVO_RELEASE);
  right_servo.write(R_SERVO_RELEASE);
  left_servo.write(L_SERVO_RELEASE);

  //-create loop 2-
  xTaskCreatePinnedToCore(
      loop_2, /* Function to implement the task */
      "Task1", /* Name of the task */
      10000,  /* Stack size in words */
      NULL,  /* Task input parameter */
      0,  /* Priority of the task */
      &Task1,  /* Task handle. */
      0); /* Core where the task should run */

  //-init LEDs and blink white-
  FastLED.addLeds<WS2811, LED_PIN_L, GRB>(left_LEDs, SIDES_LED_COUNT);
  FastLED.addLeds<WS2811, LED_PIN_K, GRB>(kick_LEDs, KICK_LED_COUNT);
  FastLED.addLeds<WS2811, LED_PIN_R, GRB>(right_LEDs, SIDES_LED_COUNT);
  FastLED.addLeds<WS2811, LED_PIN_EYES, GRB>(eyes, EYES_LED_COUNT);

  for (int i = 0; i < 54; i++) {
    if (i < SIDES_LED_COUNT) right_LEDs[i] = 0x808080;
    if (i < SIDES_LED_COUNT) left_LEDs[i] = 0x808080;
    if (i < KICK_LED_COUNT) kick_LEDs[i] = 0x808080;
  }
  FastLED.show();

  delay(100);
  for (int i = 0; i < 54; i++) {
    if (i < SIDES_LED_COUNT) right_LEDs[i] = 0x000000;
    if (i < SIDES_LED_COUNT) left_LEDs[i] = 0x000000;
    if (i < KICK_LED_COUNT) kick_LEDs[i] = 0x000000;
  }
  FastLED.show();

  delay(500);
  current_song = 1;
  pianist_mes.song = 4;
  pianist_mes.sync = 1;
  esp_now_send(pianist_addr, (uint8_t *) &pianist_mes, sizeof(pianist_mes));
  openEyes(color_eyes);
  delay(1000);
  for(int i=0; i<2; i++)
  {
    closeEyes();
    openEyes(color_eyes);
  }
}

void loop()
{
  if(current_song == 1)
  {
    if((millis()-timer_music) >= 2280)
    {
      timer_music = millis();
      esp_now_send(pianist_addr, (uint8_t *) &pianist_mes, sizeof(pianist_mes));
      if(5 <= pianist_mes.sync && pianist_mes.sync <= 12);
        freedom();

      pianist_mes.sync ++;
      if(pianist_mes.sync > 12)
      {
        current_song ++;
        pianist_mes.song = 5;
        pianist_mes.sync = 1;

        cam_mes.feedback = 2;
        esp_now_send(cam_addr, (uint8_t *) &cam_mes, sizeof(cam_mes));
        while(millis() - timer_music < 2280 + 9500);  //between songs
      }
    }
  }
  if(current_song == 2)
  {
    if((millis()-timer_music) >= 2100)
    {
      timer_music = millis();
      esp_now_send(pianist_addr, (uint8_t *) &pianist_mes, sizeof(pianist_mes));
      if(1 <= pianist_mes.sync && pianist_mes.sync <= 14)
        fireball_clapping();
      if(17 <= pianist_mes.sync && pianist_mes.sync <= 23)
        fireball_drop();
      if(25 <= pianist_mes.sync && pianist_mes.sync <= 27)
        fireball_bass();
      if(29 <= pianist_mes.sync && pianist_mes.sync <= 31)
        fireball_bass();
      if(33 <= pianist_mes.sync && pianist_mes.sync <= 46)
        fireball_chill();
      pianist_mes.sync ++;
      if(pianist_mes.sync > 46)
      {
        current_song ++;
        cam_mes.feedback = 3;
        esp_now_send(cam_addr, (uint8_t *) &cam_mes, sizeof(cam_mes));
      }
    }
  }
}