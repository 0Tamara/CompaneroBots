#include <esp_now.h>
#include <WiFi.h>

byte recv_data;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  digitalWrite(2, HIGH);
  memcpy(&recv_data, incomingData, sizeof(recv_data));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Value: ");
  Serial.println(recv_data);
  Serial.println();
  delay(500);
  digitalWrite(2, LOW);
}
 
void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  pinMode(2, OUTPUT);

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}
 
void loop() {

}