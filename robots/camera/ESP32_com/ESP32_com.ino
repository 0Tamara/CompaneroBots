//messages = right shoulder, right elbow, left shoulder, left elbow
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>

#define RXD2 16
#define TXD2 17
#define CAM_BAUD 115200

int recv_data[5];
int recv_index;
int progress = 0;  //when in the performance we are
bool start_done[] = {0, 0, 1, 1};  //if the start melodies are done - pianist fast, slow, drummer fast, slow

HardwareSerial camSerial(2);

//MAC addr:
uint8_t dancer_addr[] = {0xA0, 0xDD, 0x6C, 0x0F, 0x79, 0x38};
uint8_t drummer_addr[] = {0xA0, 0xA3, 0xB3, 0xFE, 0xD7, 0xC4};
uint8_t pianist_addr[] = {0xA8, 0x42, 0xE3, 0xA8, 0x73, 0x44};
uint8_t curtains_addr[] = {0x10, 0x06, 0x1C, 0x68, 0x42, 0x7C};

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

typedef struct struct_curtains
{
  bool open;
} struct_curtains;

struct_dancer dancer_mes;
struct_musicians drummer_mes;
struct_musicians pianist_mes;
struct_curtains curtains_mes;  //0 = closed; 1 = open

esp_now_peer_info_t peerInfo;

void setup(){
  //init serial
  Serial.begin(115200);
  camSerial.begin(CAM_BAUD, SERIAL_8N1, RXD2, TXD2);
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
  memcpy(peerInfo.peer_addr, curtains_addr, 6);
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
  curtains_mes.open = 0;

  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
}

void loop(){
  if(camSerial.available())
  {
    for(int i=0; i<5; i++)
      recv_data[i] = 0;
    recv_index = 0;
    while(camSerial.available())
    {
      recv_data[recv_index] = camSerial.read();
      recv_index ++;
    }

    for(int i=0; i<5; i++)
    {
      Serial.print(recv_data[i]);
      Serial.print(" ");
    }
    Serial.println();

    if(recv_data[1] == 0 && recv_data[2] == 0 && recv_data[3] == 0)
    {
      switch(recv_data[0])
      {
        case 1:
          //start
          dancer_mes.value = 1;
          esp_now_send(dancer_addr, (uint8_t *) &dancer_mes, sizeof(dancer_mes));
          drummer_mes.song = 1;
          esp_now_send(drummer_addr, (uint8_t *) &drummer_mes, sizeof(drummer_mes));
          delay(1000);
          pianist_mes.song = 1;
          esp_now_send(pianist_addr, (uint8_t *) &pianist_mes, sizeof(pianist_mes));
          progress = 1;
          Serial.println("Starting performance");
          break;
        case 2:
          //fast
          if(!start_done[0])
          {
            pianist_mes.song = 2;
            esp_now_send(pianist_addr, (uint8_t *) &pianist_mes, sizeof(pianist_mes));
            start_done[0] = 1;
            start_done[2] = 0;
            Serial.println("Pianist playing fast");
          }
          if(!start_done[2])
          {
            drummer_mes.song = 2;
            esp_now_send(drummer_addr, (uint8_t *) &drummer_mes, sizeof(drummer_mes));
            start_done[2] = 1;
            Serial.println("Drummer playing fast");
          }
          break;
        case 3:
          //slow
          if(!start_done[1])
          {
            pianist_mes.song = 3;
            esp_now_send(pianist_addr, (uint8_t *) &pianist_mes, sizeof(pianist_mes));
            start_done[1] = 1;
            start_done[3] = 0;
            Serial.println("Pianist playing slow");
          }
          if(!start_done[3])
          {
            drummer_mes.song = 3;
            esp_now_send(drummer_addr, (uint8_t *) &drummer_mes, sizeof(drummer_mes));
            start_done[3] = 1;
            Serial.println("Drummer playing slow");
          }
          break;
      }
    } else
    {  //sending data to dancer
      dancer_mes.r_shoulder = recv_data[0];
      dancer_mes.r_elbow = recv_data[1];
      dancer_mes.l_shoulder = recv_data[2];
      dancer_mes.l_elbow = recv_data[3];
      dancer_mes.movement = recv_data[4];
      esp_now_send(dancer_addr, (uint8_t *) &dancer_mes, sizeof(dancer_mes));
      Serial.printf("Dancer dancing %d %d %d %d %d\n", recv_data[0], recv_data[1], recv_data[2], recv_data[3], recv_data[4]);
    }
    Serial.println();
  }

  if(start_done[0] && start_done[1] && start_done[2] && start_done[3] && progress < 2)
  {
    delay(5000);
    //---music starts---
    drummer_mes.song = 4;
    esp_now_send(drummer_addr, (uint8_t *) &drummer_mes, sizeof(drummer_mes));
    dancer_mes.value = 2;
    esp_now_send(dancer_addr, (uint8_t *) &dancer_mes, sizeof(dancer_mes));
    curtains_mes.open = 1;
    esp_now_send(curtains_addr, (uint8_t *) &curtains_mes, sizeof(curtains_mes));
    camSerial.write(5);
    progress = 2;
    Serial.println("Music starting");
  }
}