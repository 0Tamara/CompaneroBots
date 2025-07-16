#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>

#define MOTOR_1 12
#define MOTOR_2 13

typedef struct struct_recv
{
  bool open;
} struct_recv;
struct_recv recv_data;

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len)
{
  memcpy(&recv_data, incomingData, sizeof(recv_data));
  Serial.print("Data: ");
  Serial.println(recv_data.open);

  if(recv_data.open)
  {
    Serial.println("opening");
    digitalWrite(MOTOR_1, HIGH);
    digitalWrite(MOTOR_2, LOW);
    delay(2000);
    digitalWrite(MOTOR_1, LOW);
    digitalWrite(MOTOR_2, LOW);
  } else
  {
    Serial.println("closing");
    digitalWrite(MOTOR_1, LOW);
    digitalWrite(MOTOR_2, HIGH);
    delay(2000);
    digitalWrite(MOTOR_1, LOW);
    digitalWrite(MOTOR_2, LOW);
  }
}

void setup()
{
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
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  //register recieve callback
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
  
  pinMode(MOTOR_1, OUTPUT);
  digitalWrite(MOTOR_1, LOW);
  pinMode(MOTOR_2, OUTPUT);
  digitalWrite(MOTOR_2, LOW);
}

void loop()
{
}
