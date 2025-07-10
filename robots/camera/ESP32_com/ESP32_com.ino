//messages = right shoulder, right elbow, left shoulder, left elbow
#include <WiFi.h>
#include <esp_now.h>

#define RXD2 16
#define TXD2 17
#define CAM_BAUD 115200

int recvData;

HardwareSerial camSerial(2);

//MAC addr:
uint8_t dancer_addr[] = {0xA0, 0xDD, 0x6C, 0x0F, 0x79, 0x38};
uint8_t drummer_addr[] = {0xA0, 0xA3, 0xB3, 0xFE, 0xD7, 0xC4};
uint8_t pianist_addr[] = {0xA8, 0x42, 0xE3, 0xA8, 0x73, 0x44};
uint8_t curtains_addr[] = {0x10, 0x06, 0x1C, 0x68, 0x42, 0x7C};

typedef struct struct_dancer
{
  byte value
  byte param1
  byte param2
  byte param3
  byte param4
} struct_dancer;

typedef struct struct_musicians
{
  byte song;
  byte sync;
} struct_musicians;

struct_dancer dancer_mes;
struct_musicians drummer_mes;
struct_musicians pianist_mes;
bool curtains_mes = 0;  //0 = closed; 1 = open

esp_now_peer_info_t peerInfo;

void setup(){
  //init serial
  Serial.begin(115200);
  camSerial.begin(CAM_BAUD, SERIAL_8N1, RXD2, TXD2);
  //init esp-now
  WiFi.mode(WIFI_STA);
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
  memcpy(peerInfo.peer_addr, curtains_addr, 6);
  if(esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }

  pinMode(2, OUTPUT);

  dancer_mes.value = 0;
  dancer_mes.param1 = 0;
  dancer_mes.param2 = 0;
  dancer_mes.param3 = 0;
  dancer_mes.param4 = 0;
  drummer_mes.song = 0;
  drummer_mes.sync = 0;
  pianist_mes.song = 0;
  pianist_mes.sync = 0;
}

void loop(){
  /*if(camSerial.available())
  {
    while (camSerial.available()){
      digitalWrite(2, HIGH);
      recvData = camSerial.read();
      Serial.print(recvData);
      Serial.print(" ");
    }
  Serial.println();
  }
  delay(100);
  digitalWrite(2, LOW);*/

  
  esp_now_send(broadcastAddress1, (uint8_t *) &time_mess, sizeof(time_mess));
}