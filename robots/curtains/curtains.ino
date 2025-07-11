#include <WiFi.h>
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

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
  
  pinMode(MOTOR_1, OUTPUT);
  digitalWrite(MOTOR_1, LOW);
  pinMode(MOTOR_2, OUTPUT);
  digitalWrite(MOTOR_2, LOW);
}

void loop()
{
}
