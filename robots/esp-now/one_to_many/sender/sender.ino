/*
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Complete project details at https://RandomNerdTutorials.com/esp-now-esp32-arduino-ide/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/
#include <esp_now.h>
#include <WiFi.h>

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress1[] = {0xE8, 0x6B, 0xEA, 0xC3, 0x0B, 0xB0};  //external esp
uint8_t broadcastAddress2[] = {0xA0, 0xDD, 0x6C, 0x0E, 0xFA, 0xA8};  //external esp
//uint8_t broadcastAddress2[] = {0xA8, 0x42, 0xE3, 0xA8, 0xE4, 0x10};  //drummer

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message_1 {
  bool on; //time during being on/off
  int time; //ms
} struct_message_1;

// Create a struct_message called myData
struct_message_1 time_mess;

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  
  // Add peer 1
  memcpy(peerInfo.peer_addr, broadcastAddress1, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  // Add peer 2
  memcpy(peerInfo.peer_addr, broadcastAddress2, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}
 
void loop() {
  if(Serial.available())
  {
    time_mess.time = Serial.read() - 0x30;
    while(Serial.available())
    {
      time_mess.time *= 10;
      time_mess.time += Serial.read() - 0x30;
    }
    Serial.println(time_mess.time);
    time_mess.on = !time_mess.on;
    esp_now_send(broadcastAddress1, (uint8_t *) &time_mess, sizeof(time_mess));
  }
  // Send message via ESP-NOW
  //esp_now_send(broadcastAddress1, (uint8_t *) &myData, sizeof(myData));
  //esp_now_send(broadcastAddress2, (uint8_t *) &myData, sizeof(myData));
}