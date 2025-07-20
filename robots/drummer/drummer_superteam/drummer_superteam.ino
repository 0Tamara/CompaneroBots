#include <ESP32Servo.h>
#include <FastLED.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>

#define R_ARM_PIN 13  //down - 0
#define L_ARM_PIN 12  //down - 90

Servo r_arm;  //0-80 = down-front
Servo l_arm;  //80-0 = down-front

#define LED_PIN_EYES 23
#define LED_COUNT_EYES 50
CRGB eyes[LED_COUNT_EYES];

uint color_eyes = 0x400040;

typedef struct struct_mes
{
  byte value;
} struct_mes;
struct_mes recv_data;

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

void OnDataRecv(const uint8_t * mac, const uint8_t *incoming_data, int len) {
  memcpy(&recv_data, incoming_data, sizeof(recv_data));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("data: ");
  Serial.println(recv_data.value);

  if(recv_data.song == 10)
  {
    openEyes(color_eyes);
    r_arm.write(80);
    l_arm.write(0);
  }
}

//---main code---
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
  //-register recieve callback-
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

  //-init servos-
  r_arm.attach(R_ARM_PIN);
  l_arm.attach(L_ARM_PIN);
  
  r_arm.write(0);
  l_arm.write(80);
  FastLED.addLeds<WS2811, LED_PIN_EYES, GRB>(eyes, LED_COUNT_EYES);
  for (int i = 0; i < 50; i++)
    eyes[i] = 0x000000;
  FastLED.show();
}

void loop()
{
}
