#include <ESP32Servo.h>
#include <esp_now.h>
#include <WiFi.h>



#define R_ARM_PIN   13
#define L_ARM_PIN   12
#define R_ELBOW_PIN 14
#define L_ELBOW_PIN 27

const int min_delay = 5; 
const int max_delay = 20;

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  byte value;
  byte rightShoulder;
  byte rightElbow;
  byte leftShoulder;
  byte leftElbow;
} struct_message;

// Create a struct_message called myData
struct_message myData;
Servo r_arm, l_arm, r_elbow, l_elbow;

void servoRamp(byte end)
{
  int t;
  byte start = r_elbow.read()+1;
  unsigned long timer;
  if(start < end)
  {
    for(int i=start; i<=end; i++)
    {
      timer = micros();
      if(i<((end-start)/2)+start)
        t = map(i, start, (start+end)/2, max_delay, min_delay);
      else
        t = map(i, (start+end)/2, end, min_delay, max_delay);

      r_elbow.write(i);
      delay(t);
      Serial.printf("position: %d\tdelay: %d\n", i, t);
    }
  } else
  {
    for(int i=start; i>=end; i--)
    {
      if(i>((start+end)/2))
        t = map(i, start, (start+end)/2, max_delay, min_delay);
      else
        t = map(i, (start+end)/2, end, min_delay, max_delay);

      r_elbow.write(i);
      delay(t);
      Serial.printf("position: %d\tdelay: %d\n", i, t);
    }
  }
  Serial.println();
}

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.printf("Daco sa prijalo, left elbow: %d, left shoulder: %d, right elbow: %d, rightShoulder: %d  \n", myData.leftElbow, myData.leftShoulder, myData.rightElbow, myData.rightShoulder);
}

void setup()
{
  Serial.begin(115200);

  r_arm.attach(R_ARM_PIN);
  l_arm.attach(L_ARM_PIN);
  r_elbow.attach(R_ELBOW_PIN);
  l_elbow.attach(L_ELBOW_PIN);

  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
  delay(1000);
}

void loop()
{
}
