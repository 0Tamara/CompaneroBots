#include <ESP32Servo.h>
#include <FastLED.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>

#define R_ARM_PIN 13
#define L_ARM_PIN 12
#define KICK_PIN 32

#define R_UP 80
#define R_DOWN 70
#define L_UP 0
#define L_DOWN 10
#define K_UP 85
#define K_DOWN 90

Servo r_arm;  //0-80 = down-front
Servo l_arm;  //80-0 = down-front
Servo kick;   //85-90-85 = kick

#define LED_PIN_L 27     //left drum
#define LED_COUNT_L 36
CRGB left_ring[LED_COUNT_L];
#define LED_PIN_K 33    //kick drum
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
unsigned long timer_music;
int kicks;
int snares;

uint color_eyes = 0xFF00FF;
uint color_palette[6] = {0xFF0000,  //colors cycling over
                        0x808000,
                        0x00FF00,
                        0x008080,
                        0x0000FF,
                        0x800080};
int color_index_kick = 0;
int color_index_left = 0;
int color_index_right = 0;

TaskHandle_t Task1;
int rising_color[3] = {0, 0, 0};  //color used for rising effects (during music)
bool blink_drums[3] = {0, 0, 0};  //rising effects active
int LEDs_pos[3] = {0, 0, LED_COUNT_R-1};  //position on the LED ring
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
typedef struct struct_cam
{
  byte feedback;
} struct_cam;

struct_mes pianist_mes;
struct_mes recv_data;
struct_cam cam_mes;

// Zapína LED na všetkých pásikoch naraz
void ledky_vedlajsie(uint color) {
  for (int i = 0; i < LED_COUNT_R; i++) {
    left_ring[i] = color;
    right_ring[i] = color;

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
void kick_ring_bubon(uint color) {
  for (int i = 0; i < LED_COUNT_K/2; i++) {
    kick_ring[i] = color;
    kick_ring[(LED_COUNT_K-1)-i] = color;
    FastLED.show();
    delay(50);
  }
  for (int i = LED_COUNT_K - 1; i >= 0; i--) {
    kick_ring[i] = 0x000000;
    kick_ring[(LED_COUNT_K-1)-i] = 0x000000;
    FastLED.show();
    delay(50);
  }
}

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
  kicks = 0;
  snares = 0;
  while(kicks < 4)
  {
    if((millis()-timer_kick) >= 570)
    {
      timer_kick = millis();
      kick.write(K_DOWN);
      changeColorsKick();
      delay(100);
      kick.write(K_UP);
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
        kick.write(K_DOWN);
        r_arm.write(R_DOWN);
        l_arm.write(L_DOWN);
        changeColorsKick();
        changeColorsRight();
        changeColorsLeft();
        delay(100);
        kick.write(K_UP);
        r_arm.write(R_UP);
        l_arm.write(L_UP);
      }
      else
      {
        if(kicks == 1)
        {
          kick.write(K_DOWN);
          r_arm.write(R_DOWN);
          changeColorsKick();
          changeColorsRight();
          delay(100);
          kick.write(K_UP);
          r_arm.write(R_UP);
          while((millis()-timer_kick) < 240) delay(10);
          l_arm.write(L_DOWN);
          changeColorsLeft();
          delay(100);
          l_arm.write(L_UP);
        }
        else
        {
          kick.write(K_DOWN);
          l_arm.write(L_DOWN);
          changeColorsKick();
          changeColorsLeft();
          delay(100);
          kick.write(K_UP);
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
      kick.write(K_DOWN);
      changeColorsKick();
      delay(100);
      kick.write(K_UP);
      kicks ++;
    }
  }
}


void fireball_chill()
{
  timer_kick = millis();
  kick.write(K_DOWN);
  r_arm.write(R_DOWN);
  l_arm.write(L_DOWN);
  changeColorsKick();
  changeColorsRight();
  changeColorsLeft();
  delay(100);
  kick.write(K_UP);
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
}

void firstLeds(uint color)
{
  ledky_vedlajsie(color);
  kick_ring_bubon(color);
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
    ledky_vedlajsie(0xFF0000);
    cam_mes.feedback = 1;
    esp_now_send(cam_addr, (uint8_t *) &cam_mes, sizeof(cam_mes));
  }
  if(recv_data.song == 3)
  {
    kick_ring_bubon(0x00FF00);
    cam_mes.feedback = 1;
    esp_now_send(cam_addr, (uint8_t *) &cam_mes, sizeof(cam_mes));
  }
  if(recv_data.song == 4)
  {
    current_song = 1;
    pianist_mes.song = 4;
    pianist_mes.sync = 1;
    for (int i = 0; i < LED_COUNT_R; i++)
      right_ring[i] = 0xFF0000;
    FastLED.show();
  }
}

//---main code---
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
          miss_out[0] = 0;
        }
      }
    }

    if(blink_drums[1])  //kick effect active
    {
      if(rising[1])  //lighting up
      {
        kick_ring[LEDs_pos[1]] = rising_color[1];
        kick_ring[LEDs_pos[1]+1] = rising_color[1];
        kick_ring[(LED_COUNT_K-1)-LEDs_pos[1]] = rising_color[1];  //going from both sides to middle
        kick_ring[(LED_COUNT_K-1)-LEDs_pos[1]-1] = rising_color[1];
        LEDs_pos[1] +=2;

        if(LEDs_pos[1] > LED_COUNT_K/2-1)  //prepare values for turn off stage
        {
          rising[1] = 0;
          LEDs_pos[1] = LED_COUNT_K/2-1;
        }
      }
      else  //turning off
      {
        kick_ring[LEDs_pos[1]] = 0;
        kick_ring[(LED_COUNT_K-1)-LEDs_pos[1]] = 0;
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
  r_arm.attach(R_ARM_PIN);
  l_arm.attach(L_ARM_PIN);
  kick.attach(KICK_PIN);
  
  kick.write(K_UP);
  r_arm.write(R_UP);
  l_arm.write(L_UP);

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
  FastLED.addLeds<WS2811, LED_PIN_L, GRB>(left_ring, LED_COUNT_L);
  FastLED.addLeds<WS2811, LED_PIN_K, GRB>(kick_ring, LED_COUNT_K);
  FastLED.addLeds<WS2811, LED_PIN_R, GRB>(right_ring, LED_COUNT_R);
  FastLED.addLeds<WS2811, LED_PIN_EYES, GRB>(eyes, LED_COUNT_EYES);

  for (int i = 0; i < 54; i++) {
    if (i < LED_COUNT_R) right_ring[i] = 0x808080;
    if (i < LED_COUNT_L) left_ring[i] = 0x808080;
    if (i < LED_COUNT_K) kick_ring[i] = 0x808080;
    if (i < LED_COUNT_EYES) eyes[i] = 0x808080;
  }
  FastLED.show();

  delay(100);
  for (int i = 0; i < 54; i++) {
    if (i < LED_COUNT_R) right_ring[i] = 0x000000;
    if (i < LED_COUNT_L) left_ring[i] = 0x000000;
    if (i < LED_COUNT_K) kick_ring[i] = 0x000000;
    if (i < LED_COUNT_EYES) eyes[i] = 0x000000;
  }
  FastLED.show();
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
