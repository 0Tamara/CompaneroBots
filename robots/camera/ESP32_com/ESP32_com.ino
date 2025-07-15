//messages = right shoulder, right elbow, left shoulder, left elbow
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>

#define RXD2 16
#define TXD2 17
#define CAM_BAUD 115200

int serial_data[5];
int recv_index;
int progress = 0;  //when in the performance we are
bool start_done[] = {0, 0, 1, 1};  //if the start melodies are done - pianist fast, slow, drummer fast, slow
bool start_playing = 0;
bool start_loading = 0;
bool dancing_sync = 0;  //dancer dancing by camera

unsigned long timer_start;

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

typedef struct struct_recv
{
  byte value;
} struct_recv;

struct_recv recv_data;

esp_now_peer_info_t peerInfo;

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&recv_data, incomingData, sizeof(recv_data));
  Serial.printf("Recv data: %d\n", recv_data.value);

  switch(recv_data.value)
  {
    case 1:  //end of starting melody
      start_playing = 0;
      break;
    case 2:  //eng of freedom
      camSerial.write(5);  //start scanning camera and playing music
      dancer_mes.value = 3;  //dancer will dance by camera
      break;
  }
}

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
  //register recieve callback
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

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
      serial_data[i] = 0;
    recv_index = 0;
    while(camSerial.available())
    {
      serial_data[recv_index] = camSerial.read();
      recv_index ++;
    }

    Serial.print("camSerial recieved: ");
    for(int i=0; i<5; i++)
    {
      Serial.print(serial_data[i]);
      Serial.print(" ");
    }
    Serial.println();

    if(serial_data[1] == 0 && serial_data[2] == 0 && serial_data[3] == 0)  //if there's only one number sent
    {
      switch(serial_data[0])
      {
        case 1:
          //start
          dancer_mes.value = 1;
          drummer_mes.song = 1;
          pianist_mes.song = 1;
          esp_now_send(dancer_addr, (uint8_t *) &dancer_mes, sizeof(dancer_mes));
          esp_now_send(drummer_addr, (uint8_t *) &drummer_mes, sizeof(drummer_mes));
          esp_now_send(pianist_addr, (uint8_t *) &pianist_mes, sizeof(pianist_mes));
          progress = 1;
          start_loading = 1;
          start_playing = 1;
          timer_start = millis();
          Serial.println("Starting performance");
          break;
        case 2:
          //fast
          if(!start_done[0] && !start_playing)
          {
            pianist_mes.song = 2;
            esp_now_send(pianist_addr, (uint8_t *) &pianist_mes, sizeof(pianist_mes));
            start_playing = 1;
            start_done[0] = 1;
            start_done[2] = 0;
            Serial.println("Pianist playing fast");
          } else if(!start_done[2] && !start_playing)
          {
            drummer_mes.song = 2;
            esp_now_send(drummer_addr, (uint8_t *) &drummer_mes, sizeof(drummer_mes));
            start_playing = 1;
            start_done[2] = 1;
            Serial.println("Drummer playing fast");
          } else if(millis()-timer_start > 5000)  //min time between starting melodies and main songs
          {
            //---music starts---
            drummer_mes.song = 4;
            esp_now_send(drummer_addr, (uint8_t *) &drummer_mes, sizeof(drummer_mes));
            dancer_mes.value = 2;
            esp_now_send(dancer_addr, (uint8_t *) &dancer_mes, sizeof(dancer_mes));
            curtains_mes.open = 1;
            esp_now_send(curtains_addr, (uint8_t *) &curtains_mes, sizeof(curtains_mes));
            progress = 2;
            Serial.println("Freedom starting");
          }
          break;
        case 3:
          //slow
          if(!start_done[1] && !start_playing)
          {
            pianist_mes.song = 3;
            esp_now_send(pianist_addr, (uint8_t *) &pianist_mes, sizeof(pianist_mes));
            start_playing = 1;
            start_done[1] = 1;
            start_done[3] = 0;
            Serial.println("Pianist playing slow");
          } else if(!start_done[3] && !start_playing)
          {
            drummer_mes.song = 3;
            esp_now_send(drummer_addr, (uint8_t *) &drummer_mes, sizeof(drummer_mes));
            start_playing = 1;
            start_done[3] = 1;
            Serial.println("Drummer playing slow");
          } else if(millis()-timer_start > 5000)  //min time between starting melodies and main songs
          {
            //---music starts---
            drummer_mes.song = 4;
            esp_now_send(drummer_addr, (uint8_t *) &drummer_mes, sizeof(drummer_mes));
            dancer_mes.value = 2;
            esp_now_send(dancer_addr, (uint8_t *) &dancer_mes, sizeof(dancer_mes));
            curtains_mes.open = 1;
            esp_now_send(curtains_addr, (uint8_t *) &curtains_mes, sizeof(curtains_mes));
            progress = 2;
            Serial.println("Freedom starting");
          }
          break;
      }
    } else
    {  //sending data to dancer
      if(!start_playing)
      {
        dancer_mes.r_shoulder = serial_data[0];
        dancer_mes.r_elbow = serial_data[1];
        dancer_mes.l_shoulder = serial_data[2];
        dancer_mes.l_elbow = serial_data[3];
        dancer_mes.movement = serial_data[4];
        esp_now_send(dancer_addr, (uint8_t *) &dancer_mes, sizeof(dancer_mes));
        Serial.printf("Dancer dancing %d %d %d %d %d\n", serial_data[0], serial_data[1], serial_data[2], serial_data[3], serial_data[4]);
      }
    }
    Serial.println();
  }

  if(start_done[0] && start_done[1] && start_done[2] && start_done[3] && progress < 2 && !start_playing)  //all start melodies are done
    timer_start = millis();

  if(start_loading && millis() - timer_start > 2000)  //time between the startup and measuring the sensor
  {
    start_loading = 0;
    start_playing = 0;
  }
}