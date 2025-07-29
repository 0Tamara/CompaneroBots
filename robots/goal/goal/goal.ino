#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>
#include <VL53L0X.h>

#define THR_DISTANCE 1000

VL53L0X distanceSensor;
int vzdialenost;

//MAC addr:
uint8_t dancer_addr[] = {0xA0, 0xDD, 0x6C, 0x0F, 0x79, 0x38};
uint8_t drummer_addr[] = {0xA0, 0xA3, 0xB3, 0xFE, 0xD7, 0xC4};
uint8_t pianist_addr[] = {0x84, 0x0D, 0x8E, 0xE4, 0xB4, 0x58};

typedef struct struct_com
{
  byte value;
} struct_com;

struct_com message;

esp_now_peer_info_t peerInfo;


void setup(){
  //debug purpose
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  //init WiFi & read MAC address
  WiFi.mode(WIFI_STA);
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  Serial.printf("My MAC address: {0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X}\n",
                baseMac[0], baseMac[1], baseMac[2],
                baseMac[3], baseMac[4], baseMac[5]);
  //init esp-now
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  //register peers
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  //add peers
  memcpy(peerInfo.peer_addr, dancer_addr, 6);
  if(esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }
  memcpy(peerInfo.peer_addr, drummer_addr, 6);
  if(esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }
  memcpy(peerInfo.peer_addr, pianist_addr, 6);
  if(esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }

  //init ToF sensor
  Wire.begin();
  distanceSensor.setTimeout(100);
  if (!distanceSensor.init())
    Serial.println("Failed to detect and initialize sensor VL53L0X !");

  message.value = 0;

}

void loop(){
  vzdialenost = distanceSensor.readRangeSingleMillimeters();
  if(vzdialenost < THR_DISTANCE)
  {
    Serial.println(vzdialenost);
    digitalWrite(2, HIGH);
    message.value = 10;
    esp_now_send(NULL, (uint8_t *) &message, sizeof(message));
    delay(100);
    digitalWrite(2, LOW);
    delay(100);
  }
}