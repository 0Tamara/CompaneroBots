#include <WiFi.h>
#include <esp_now.h>
#include <ESP32Servo.h>

 //motors
#define RR_EN 18  // Right rear enable pin
const int RR_DIR[] = {17, 5}; // Right rear direction pins
#define LR_EN 2  // Left rear enable pin
const int LR_DIR[] = {16, 4}; // Left rear direction pins
#define RF_EN 14  // Right front enable pin
const int RF_DIR[] = {26, 27}; // Right front direction pins
#define LF_EN 32   // Left front enable pin
const int LF_DIR[] = {33, 25}; // Left front direction pins

//communication
uint8_t cam_addr[] = {0xC0, 0x49, 0xEF, 0xD0, 0x8C, 0xC0};  //camera esp MAC addr
esp_now_peer_info_t peer_info;

typedef struct struct_cam
{
  byte feedback;
} struct_cam;
typedef struct struct_mes
{
  byte value;
  byte r_shoulder;
  byte r_elbow;
  byte l_shoulder;
  byte l_elbow;
  byte movement;
} struct_mes;

struct_mes recv_data;
struct_cam cam_mes;

const int min_delay = 5; 
const int max_delay = 20;
Servo rightShoulder, leftShoulder, rightElbow, leftElbow;

void servoRamp(byte end, Servo& servo)
{
  int t;
  byte start = servo.read()+1;
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

      servo.write(i);
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

      servo.write(i);
      delay(t);
      Serial.printf("position: %d\tdelay: %d\n", i, t);
    }
  }
  Serial.println();
}


unsigned long timer_reset;

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&recv_data, incomingData, sizeof(recv_data));
  if (recv_data.value == 1)
  {
    //zapneme ledky
    Serial.printf("Prijala sa: %d\n", recv_data.value);
    //natvrdo tancuje
    arms_up();
    delay(1000);
    arms_horizontally();
    delay(1000);
    arms_down();
    delay(1000);
    wave();
    delay(1000);
    wave2();
    delay(1000);
    askew();
    delay(1000);
    askew2();
    delay(1000);
    right_hip();
    delay(1000);
    left_hip();
    delay(1000);
    head();
    delay(1000);
    waving();
    delay(1000);
  }
  if (recv_data.value == 2) 
  {
    Serial.printf("Prijala sa: %d, spustam natvrdo pohyb\n", recv_data.value);
  
  }
  if(recv_data.value == 3)
  {
    Serial.printf("Daco sa prijalo, left elbow: %d, left shoulder: %d, right elbow: %d, right shoulder: %d, movement: %d \n", 
                  recv_data.l_elbow, recv_data.l_shoulder, recv_data.r_elbow, 
                  recv_data.r_shoulder, recv_data.movement);
    servoRamp(180 - recv_data.r_shoulder, rightShoulder);
    servoRamp(70 + recv_data.r_elbow, rightElbow); // toto je 70 pri nule
    servoRamp(recv_data.l_elbow, leftElbow); 
    servoRamp(70 + recv_data.l_shoulder, leftShoulder);  
  } 
  
  
}
void setup() {
  Serial.begin(115200);
  // communication
  WiFi.mode(WIFI_STA);  //set wifi to station
  //-init esp-now-
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  //-register peer-
  peer_info.channel = 0;  
  peer_info.encrypt = false;

  //-add peer-
  memcpy(peer_info.peer_addr, cam_addr, 6);
  if (esp_now_add_peer(&peer_info) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }
  //-register recieve callback-
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

  // hardware init
  rightShoulder.attach(13);
  rightElbow.attach(14);
  leftShoulder.attach(12);
  leftElbow.attach(27);

  for(int i=0; i<2; i++) {
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
void forward(byte speed)
{
  digitalWrite(RF_DIR[0], LOW);
  digitalWrite(RF_DIR[1], HIGH);
  ledcWrite(RF_EN, speed/2);

  digitalWrite(LF_DIR[0], LOW);
  digitalWrite(LF_DIR[1], HIGH);
  ledcWrite(LF_EN, speed/2);

  digitalWrite(RR_DIR[0], LOW);
  digitalWrite(RR_DIR[1], HIGH);
  ledcWrite(RR_EN, speed);

  digitalWrite(LR_DIR[0], LOW);
  digitalWrite(LR_DIR[1], HIGH);
  ledcWrite(LR_EN, speed/2);
}
void backward(byte speed)
{
  digitalWrite(RF_DIR[0], HIGH);
  digitalWrite(RF_DIR[1], LOW);
  ledcWrite(RF_EN, speed/2);

  digitalWrite(LF_DIR[0], HIGH);
  digitalWrite(LF_DIR[1], LOW);
  ledcWrite(LF_EN, speed/2);

  digitalWrite(RR_DIR[0], HIGH);
  digitalWrite(RR_DIR[1], LOW);
  ledcWrite(RR_EN, speed);

  digitalWrite(LR_DIR[0], HIGH);
  digitalWrite(LR_DIR[1], LOW);
  ledcWrite(LR_EN, speed/2);
}
void left(byte speed)
{
  digitalWrite(RF_DIR[0], LOW);
  digitalWrite(RF_DIR[1], HIGH);
  ledcWrite(RF_EN, speed/2);

  digitalWrite(LF_DIR[0], HIGH);
  digitalWrite(LF_DIR[1], LOW);
  ledcWrite(LF_EN, speed);

  digitalWrite(RR_DIR[0], LOW);
  digitalWrite(RR_DIR[1], HIGH);
  ledcWrite(RR_EN, speed/2);

  digitalWrite(LR_DIR[0], HIGH);
  digitalWrite(LR_DIR[1], LOW);
  ledcWrite(LR_EN, speed/2);
}
void right(byte speed)
{
  digitalWrite(RF_DIR[0], HIGH);
  digitalWrite(RF_DIR[1], LOW);
  ledcWrite(RF_EN, speed/2);

  digitalWrite(LF_DIR[0], LOW);
  digitalWrite(LF_DIR[1], HIGH);
  ledcWrite(LF_EN, speed);

  digitalWrite(RR_DIR[0], HIGH);
  digitalWrite(RR_DIR[1], LOW);
  ledcWrite(RR_EN, speed/2);

  digitalWrite(LR_DIR[0], LOW);
  digitalWrite(LR_DIR[1], HIGH);
  ledcWrite(LR_EN, speed/2);
}
void stop()
{
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
  analogWrite(RF_EN, speed/2);

  digitalWrite(LF_DIR[0], HIGH);
  digitalWrite(LF_DIR[1], LOW);
  analogWrite(LF_EN, speed/2);

  digitalWrite(RR_DIR[0], HIGH);
  digitalWrite(RR_DIR[1], LOW);
  analogWrite(RR_EN, speed);

  digitalWrite(LR_DIR[0], LOW);
  digitalWrite(LR_DIR[1], HIGH);
  analogWrite(LR_EN, speed/2);
}

void to_right_side(int speed) {
  digitalWrite(RF_DIR[0], HIGH);
  digitalWrite(RF_DIR[1], LOW);
  analogWrite(RF_EN, speed/2);

  digitalWrite(LF_DIR[0], LOW);
  digitalWrite(LF_DIR[1], HIGH);
  analogWrite(LF_EN, speed/2);

  digitalWrite(RR_DIR[0], LOW);
  digitalWrite(RR_DIR[1], HIGH);
  analogWrite(RR_EN, speed);

  digitalWrite(LR_DIR[0], HIGH);
  digitalWrite(LR_DIR[1], LOW);
  analogWrite(LR_EN, speed/2);
}

//servo movement 

void arms_down(){
  rightShoulder.write(180);
  rightElbow.write(70);
  leftShoulder.write(0);
  leftElbow.write(0);
}

void arms_horizontally(){
  rightShoulder.write(90);
  rightElbow.write(70);
  leftShoulder.write(90);
  leftElbow.write(0);
}

void arms_up(){
  rightShoulder.write(10);
  rightElbow.write(70);
  leftShoulder.write(150);
  leftElbow.write(0);
}

void wave(){
  rightShoulder.write(70);
  rightElbow.write(20);
  leftShoulder.write(90);
  leftElbow.write(0);
}

void wave2(){
  rightShoulder.write(90);
  rightElbow.write(70);
  leftShoulder.write(110);
  leftElbow.write(50);
}

void askew(){
  rightShoulder.write(160);
  rightElbow.write(70);
  leftShoulder.write(150);
  leftElbow.write(0);
}

void askew2(){
  rightShoulder.write(30);
  rightElbow.write(70);
  leftShoulder.write(20);
  leftElbow.write(0);
}

void right_hip(){
  rightShoulder.write(140);
  rightElbow.write(120);
  leftShoulder.write(150);
  leftElbow.write(0);
}

void left_hip(){
  rightShoulder.write(10);
  rightElbow.write(70);
  leftShoulder.write(40);
  leftElbow.write(50);
}

void head(){
  rightShoulder.write(60);
  rightElbow.write(20);
  leftShoulder.write(120);
  leftElbow.write(0);
}

void waving(){
  rightShoulder.write(50);
  rightElbow.write(70);
  leftShoulder.write(0);
  leftElbow.write(0);
  delay(800);
  rightElbow.write(30);
  delay(800);
  rightElbow.write(110);
}