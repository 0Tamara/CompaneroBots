#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>

int recv_index;
int progress = 0;  //when in the performance we are

unsigned long timer_start;

HardwareSerial camSerial(2);

//MAC addr:
uint8_t dancer_addr[] = {0xA0, 0xDD, 0x6C, 0x0F, 0x79, 0x38};
uint8_t drummer_addr[] = {0xA0, 0xA3, 0xB3, 0xFE, 0xD7, 0xC4};
uint8_t pianist_addr[] = {0x84, 0x0D, 0x8E, 0xE4, 0xB4, 0x58};

typedef struct struct_dancer
{
  byte value;
  byte r_shoulder;
  byte r_elbow;
  byte l_shoulder;
  byte l_elbow;
  byte movement;
} struct_dancer;

typedef struct struct_musicians
{
  byte song;
  byte sync;
} struct_musicians;

struct_dancer dancer_mes;
struct_musicians drummer_mes;
struct_musicians pianist_mes;

typedef struct struct_recv
{
  byte value;
} struct_recv;

struct_recv recv_data;

esp_now_peer_info_t peerInfo;


void setup(){
  //init serial
  Serial.begin(115200);
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

  dancer_mes.value = 0;
  dancer_mes.r_shoulder = 0;
  dancer_mes.r_elbow = 0;
  dancer_mes.l_shoulder = 0;
  dancer_mes.l_elbow = 0;
  dancer_mes.movement = 0;
  drummer_mes.song = 0;
  drummer_mes.sync = 1;
  pianist_mes.song = 0;
  pianist_mes.sync = 1;

  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
}

void loop(){
  pianist_mes.song = 1;
  esp_now_send(pianist_addr, (uint8_t *) &pianist_mes, sizeof(pianist_mes));
  digitalWrite(2, LOW);
  delay(2000);
  pianist_mes.song = 2;
  esp_now_send(pianist_addr, (uint8_t *) &pianist_mes, sizeof(pianist_mes));
  digitalWrite(2, HIGH);
  delay(20000);
  pianist_mes.song = 3;
  esp_now_send(pianist_addr, (uint8_t *) &pianist_mes, sizeof(pianist_mes));
  digitalWrite(2, HIGH);
  delay(20000);
}