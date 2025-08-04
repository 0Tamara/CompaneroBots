#include <WiFi.h>
#include <esp_now.h>
#include <ESP32Servo.h>
#include <FastLED.h>

//motors
#define RR_EN 19                   // Right rear enable pin
const int RR_DIR[] = { 25, 21 };   // Right rear direction pins
#define LR_EN 23                   // Left rear enable pin
const int LR_DIR[] = { 22, 26 };   // Left rear direction pins
#define RF_EN 2                    // Right front enable pin
const int RF_DIR[] = { 4, 16 };    // Right front direction pins
#define LF_EN 18                   // Left front enable pin
const int LF_DIR[] = { 5, 17 };    // Left front direction pins


#define LED_PIN_EYES 32
#define LED_COUNT_EYES 50
CRGB eyes[LED_COUNT_EYES];
TaskHandle_t Task1;
uint color_eyes = 0xFF00FF;

//communication
uint8_t cam_addr[] = { 0xC0, 0x49, 0xEF, 0xD0, 0x8C, 0xC0 };  //camera esp MAC addr
esp_now_peer_info_t peer_info;

typedef struct struct_cam {
  byte feedback;
} struct_cam;
typedef struct struct_mes {
  byte value;
  byte r_shoulder;
  byte r_elbow;
  byte l_shoulder;
  byte l_elbow;
  byte movement;
} struct_mes;

struct_mes recv_data;
struct_cam cam_mes;

const int min_delay = 0;
const int max_delay = 15;
Servo rightShoulder, leftShoulder, rightElbow, leftElbow;

//---eyes functions---
void closeEyes()  //cca 300ms
{
  // Blink LEDs in reverse order (off in sections)
  for (int i = 20; i < 25; i++) eyes[i] = 0x000000; // Left eye
  for (int i = 45; i < 50; i++) eyes[i] = 0x000000; // Right eye
  FastLED.show();

  delay(50);

  // Now let's go down the LED sections
  for (int i = 15; i < 20; i++) eyes[i] = 0x000000;
  for (int i = 40; i < 45; i++) eyes[i] = 0x000000;
  FastLED.show();

  delay(50);

  for (int i = 10; i < 15; i++) eyes[i] = 0x000000;
  for (int i = 35; i < 40; i++) eyes[i] = 0x000000;
  FastLED.show();

  delay(50);

  for (int i = 5; i < 10; i++) eyes[i] = 0x000000;
  for (int i = 30; i < 35; i++) eyes[i] = 0x000000;
  FastLED.show();

  delay(50);

  for (int i = 0; i < 5; i++) eyes[i] = 0x000000;
  for (int i = 25; i < 30; i++) eyes[i] = 0x000000;
  FastLED.show();

  delay(50);
}

void openEyes(uint color)  //cca 300ms
{
  // Blink LEDs in reverse order (turning LEDs back on)
  for (int i = 0; i < 5; i++) eyes[i] = color; // Left eye
  for (int i = 25; i < 30; i++) eyes[i] = color; // Right eye
  FastLED.show();

  delay(50);

  for (int i = 5; i < 10; i++) eyes[i] = color;
  for (int i = 30; i < 35; i++) eyes[i] = color;
  FastLED.show();

  delay(50);

  for (int i = 10; i < 15; i++) eyes[i] = color;
  for (int i = 35; i < 40; i++) eyes[i] = color;
  FastLED.show();

  delay(50);

  for (int i = 15; i < 20; i++) eyes[i] = color;
  for (int i = 40; i < 45; i++) eyes[i] = color;
  FastLED.show();

  delay(50);

  for (int i = 20; i < 25; i++) eyes[i] = color;
  for (int i = 45; i < 50; i++) eyes[i] = color;
  FastLED.show();

  delay(50);
}

//---servo functions---
void servoRamp(byte end, Servo& servo) {
  int t;
  byte start = servo.read() + 1;
  unsigned long timer;
  if (start < end) {
    for (int i = start; i <= end; i++) {
      timer = micros();
      if (i < ((end - start) / 2) + start)
        t = map(i, start, (start + end) / 2, max_delay, min_delay);
      else
        t = map(i, (start + end) / 2, end, min_delay, max_delay);

      servo.write(i);
      delay(t);
      //Serial.printf("position: %d\tdelay: %d\n", i, t);
    }
  } else {
    for (int i = start; i >= end; i--) {
      if (i > ((start + end) / 2))
        t = map(i, start, (start + end) / 2, max_delay, min_delay);
      else
        t = map(i, (start + end) / 2, end, min_delay, max_delay);

      servo.write(i);
      delay(t);
      //Serial.printf("position: %d\tdelay: %d\n", i, t);
    }
  }
  Serial.println();
}

void moveAll(byte end1, Servo& servo1, byte end2, Servo& servo2, byte end3, Servo& servo3, byte end4, Servo& servo4) {
  int t;
  byte start1 = servo1.read() + 1;
  byte start2 = servo2.read() + 1;
  byte start3 = servo3.read() + 1;
  byte start4 = servo4.read() + 1;
  byte angle1; byte angle2; byte angle3; byte angle4;
  unsigned long timer;
  for (int i = 0; i <= 64; i++)
  {
    angle1 = map(i, 0, 64, start1, end1);
    angle2 = map(i, 0, 64, start2, end2);
    angle3 = map(i, 0, 64, start3, end3);
    angle4 = map(i, 0, 64, start4, end4);
    timer = micros();
    if (i < 32)
      t = map(i, 0, 32, max_delay, min_delay);
    else
      t = map(i, 32, 64, min_delay, max_delay);

    servo1.write(angle1);
    servo2.write(angle2);
    servo3.write(angle3);
    servo4.write(angle4);
    delay(t);
  }
}


unsigned long timer_reset;
void OnDataRecv(const uint8_t* mac, const uint8_t* incomingData, int len) {
  
  memcpy(&recv_data, incomingData, sizeof(recv_data));
  if (recv_data.value == 1) {
   
  }
  if (recv_data.value == 2) {
    Serial.printf("Prijala sa: %d, spustam natvrdo pohyb\n", recv_data.value);
  }
  if (recv_data.value == 3) {
    Serial.printf("Daco sa prijalo, left elbow: %d, left shoulder: %d, right elbow: %d, right shoulder: %d, movement: %d \n",
                  recv_data.l_elbow, recv_data.l_shoulder, recv_data.r_elbow,
                  recv_data.r_shoulder, recv_data.movement);


    if(recv_data.r_elbow < 90)
      recv_data.r_elbow = 0;
    else
      recv_data.r_elbow -= 90;
    if(recv_data.l_elbow < 90)
      recv_data.l_elbow = 0;
    else
      recv_data.l_elbow -= 90;
    if(recv_data.r_elbow > 180)
      recv_data.r_elbow = 180;
    if(recv_data.l_elbow > 180)
      recv_data.l_elbow = 180;
                  
    moveAll(180 - recv_data.r_shoulder, rightShoulder, /*180 - recv_data.r_elbow*/70, rightElbow, /*180 - recv_data.l_elbow*/70, leftElbow, recv_data.l_shoulder, leftShoulder);
  }
  
}

void loop_2(void* parameter)
{
  while(true)
  {
    closeEyes();
    openEyes(color_eyes);
    delay(5000);
  }
}

void setup() {
  Serial.begin(115200);
  // communication
  
  WiFi.mode(WIFI_STA);  //set wifi to station
  //-init esp-now-
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  //-register peer-
  peer_info.channel = 0;
  peer_info.encrypt = false;

  //-add peer-
  memcpy(peer_info.peer_addr, cam_addr, 6);
  if (esp_now_add_peer(&peer_info) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  //-register recieve callback-
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
  

  //-create loop 2-
  xTaskCreatePinnedToCore(
    loop_2, /* Function to implement the task */
    "Task1", /* Name of the task */
    10000,  /* Stack size in words */
    NULL,  /* Task input parameter */
    0,  /* Priority of the task */
    &Task1,  /* Task handle. */
    0); /* Core where the task should run */

  // hardware init
  FastLED.addLeds<WS2811, LED_PIN_EYES, GRB>(eyes, LED_COUNT_EYES);
  for (int i = 0; i < 50; i++)
    eyes[i] = 0x000000;
  FastLED.show();
  rightShoulder.attach(13);
  rightElbow.attach(14);
  leftShoulder.attach(12);
  leftElbow.attach(27);
  leftElbow.attach(33);

  for (int i = 0; i < 2; i++) {
    pinMode(RR_DIR[i], OUTPUT);
    pinMode(LR_DIR[i], OUTPUT);
    pinMode(RF_DIR[i], OUTPUT);
    pinMode(LF_DIR[i], OUTPUT);
  }
  ledcAttachChannel(RR_EN, 1000, 8, 1);
  ledcAttachChannel(LR_EN, 1000, 8, 1);
  ledcAttachChannel(RF_EN, 1000, 8, 1);
  ledcAttachChannel(LF_EN, 1000, 8, 1);
}


void loop() {
}

//motor movement
void forward(byte speed) {
  digitalWrite(RF_DIR[0], LOW);
  digitalWrite(RF_DIR[1], HIGH);
  ledcWrite(RF_EN, speed / 2);

  digitalWrite(LF_DIR[0], LOW);
  digitalWrite(LF_DIR[1], HIGH);
  ledcWrite(LF_EN, speed / 2);

  digitalWrite(RR_DIR[0], LOW);
  digitalWrite(RR_DIR[1], HIGH);
  ledcWrite(RR_EN, speed);

  digitalWrite(LR_DIR[0], LOW);
  digitalWrite(LR_DIR[1], HIGH);
  ledcWrite(LR_EN, speed / 2);
}
void backward(byte speed) {
  digitalWrite(RF_DIR[0], HIGH);
  digitalWrite(RF_DIR[1], LOW);
  ledcWrite(RF_EN, speed / 2);

  digitalWrite(LF_DIR[0], HIGH);
  digitalWrite(LF_DIR[1], LOW);
  ledcWrite(LF_EN, speed / 2);

  digitalWrite(RR_DIR[0], HIGH);
  digitalWrite(RR_DIR[1], LOW);
  ledcWrite(RR_EN, speed);

  digitalWrite(LR_DIR[0], HIGH);
  digitalWrite(LR_DIR[1], LOW);
  ledcWrite(LR_EN, speed / 2);
}
void left(byte speed) {
  digitalWrite(RF_DIR[0], LOW);
  digitalWrite(RF_DIR[1], HIGH);
  ledcWrite(RF_EN, speed / 2);

  digitalWrite(LF_DIR[0], HIGH);
  digitalWrite(LF_DIR[1], LOW);
  ledcWrite(LF_EN, speed);

  digitalWrite(RR_DIR[0], LOW);
  digitalWrite(RR_DIR[1], HIGH);
  ledcWrite(RR_EN, speed / 2);

  digitalWrite(LR_DIR[0], HIGH);
  digitalWrite(LR_DIR[1], LOW);
  ledcWrite(LR_EN, speed / 2);
}
void right(byte speed) {
  digitalWrite(RF_DIR[0], HIGH);
  digitalWrite(RF_DIR[1], LOW);
  ledcWrite(RF_EN, speed / 2);

  digitalWrite(LF_DIR[0], LOW);
  digitalWrite(LF_DIR[1], HIGH);
  ledcWrite(LF_EN, speed);

  digitalWrite(RR_DIR[0], HIGH);
  digitalWrite(RR_DIR[1], LOW);
  ledcWrite(RR_EN, speed / 2);

  digitalWrite(LR_DIR[0], LOW);
  digitalWrite(LR_DIR[1], HIGH);
  ledcWrite(LR_EN, speed / 2);
}
void stop() {
  digitalWrite(RF_DIR[0], LOW);
  digitalWrite(RF_DIR[1], LOW);
  digitalWrite(LF_DIR[0], LOW);
  digitalWrite(LF_DIR[1], LOW);
  digitalWrite(RR_DIR[0], LOW);
  digitalWrite(RR_DIR[1], LOW);
  digitalWrite(LR_DIR[0], LOW);
  digitalWrite(LR_DIR[1], LOW);
}



void to_left_side(int speed) {
  digitalWrite(RF_DIR[0], LOW);
  digitalWrite(RF_DIR[1], HIGH);
  analogWrite(RF_EN, speed / 2);

  digitalWrite(LF_DIR[0], HIGH);
  digitalWrite(LF_DIR[1], LOW);
  analogWrite(LF_EN, speed / 2);

  digitalWrite(RR_DIR[0], HIGH);
  digitalWrite(RR_DIR[1], LOW);
  analogWrite(RR_EN, speed);

  digitalWrite(LR_DIR[0], LOW);
  digitalWrite(LR_DIR[1], HIGH);
  analogWrite(LR_EN, speed / 2);
}

void to_right_side(int speed) {
  digitalWrite(RF_DIR[0], HIGH);
  digitalWrite(RF_DIR[1], LOW);
  analogWrite(RF_EN, speed / 2);

  digitalWrite(LF_DIR[0], LOW);
  digitalWrite(LF_DIR[1], HIGH);
  analogWrite(LF_EN, speed / 2);

  digitalWrite(RR_DIR[0], LOW);
  digitalWrite(RR_DIR[1], HIGH);
  analogWrite(RR_EN, speed);

  digitalWrite(LR_DIR[0], HIGH);
  digitalWrite(LR_DIR[1], LOW);
  analogWrite(LR_EN, speed / 2);
}

//servo movement

void arms_down() {
  rightShoulder.write(180);
  rightElbow.write(70);
  leftShoulder.write(0);
  leftElbow.write(70);
}

void arms_horizontally() {
  rightShoulder.write(90);
  rightElbow.write(70);
  leftShoulder.write(90);
  leftElbow.write(70);
}

void arms_up() {
  rightShoulder.write(10);
  rightElbow.write(70);
  leftShoulder.write(150);
  leftElbow.write(70);
}

void wave() {
  rightShoulder.write(70);
  rightElbow.write(20);




  leftShoulder.write(90);
  leftElbow.write(70);
}

void wave2() {
  rightShoulder.write(90);
  rightElbow.write(70);
  leftShoulder.write(110);
  leftElbow.write(50);
}

void askew() {
  rightShoulder.write(160);
  rightElbow.write(70);
  leftShoulder.write(150);
  leftElbow.write(70);
}

void askew2() {
  rightShoulder.write(30);
  rightElbow.write(70);
  leftShoulder.write(20);
  leftElbow.write(70);
}

void right_hip() {
  rightShoulder.write(140);
  rightElbow.write(120);
  leftShoulder.write(150);
  leftElbow.write(70);
}

void left_hip() {
  rightShoulder.write(10);
  rightElbow.write(70);
  leftShoulder.write(40);
  leftElbow.write(50);
}

void head() {
  rightShoulder.write(60);
  rightElbow.write(20);
  leftShoulder.write(120);
  leftElbow.write(70);
}

void waving() {
  rightShoulder.write(50);
  rightElbow.write(70);
  leftShoulder.write(0);
  leftElbow.write(70);
  delay(800);
  rightElbow.write(30);
  delay(800);
  rightElbow.write(110);
}