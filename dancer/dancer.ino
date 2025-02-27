#include <BLEDevice.h>
#include <ESP32Servo.h>

#define bleServerName "Companero"  //name of the BLE server we are connecting to
//UUIDs of the service and characteristics:
static BLEUUID serviceUUID("edddc3d6-5d4a-4677-87c5-f4f7d40b6111");
static BLEUUID musicCharUUID("90c43130-b419-4bfd-bcf4-bb9c8373ddd6");

static BLEAddress* serverAddress;  //address of the device
static BLERemoteCharacteristic* music_char;  //characteristic for music sync
//notification options
const uint8_t notificationOn[] = {0x1, 0x0};
const uint8_t notificationOff[] = {0x0, 0x0};

static bool serverFound = false;
static bool connected = false;

int music_command;
bool go = 0;

Servo rd, rv, lv, ld, r, l;

 //motors
#define RR_EN 18  // Right rear enable pin
const int RR_DIR[] = {17, 5}; // Right rear direction pins
#define LR_EN 2  // Left rear enable pin
const int LR_DIR[] = {16, 4}; // Left rear direction pins
#define RF_EN 14  // Right front enable pin
const int RF_DIR[] = {26, 27}; // Right front direction pins
#define LF_EN 32   // Left front enable pin
const int LF_DIR[] = {33, 25}; // Left front direction pins

unsigned long timer_reset;

//---functions for getting notifications---
static void notifyMusic(BLERemoteCharacteristic* pBLERemoteMusicChar, uint8_t* data, size_t length, bool isNotify)
{
  memcpy(&music_command, data, sizeof(music_command));
  if(music_command == 17)
    go = 1;
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks  //when found a server
{
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    if(advertisedDevice.getName() == bleServerName)  //check if the server is the right one
    {
      advertisedDevice.getScan()->stop();  //stop scanning
      serverAddress = new BLEAddress(advertisedDevice.getAddress());  //get the server's address
      serverFound = true;
      Serial.println(" + Device found. Connecting...");
    }
  }
};
//---connect to the BLE server---
bool connectToServer(BLEAddress address)
{
  Serial.println("-+-Creating BLE client...");
  BLEClient* bleClient = BLEDevice::createClient();
  
  Serial.println("-+-Attempting connection...");
  if(!bleClient->connect(address))
  {
    Serial.println("-x-Connection failed!");
    return false;
  }
  Serial.println("-+-Connected to server, searching for services...");
  delay(500);  //time for client to initialize
  //find service
  BLERemoteService* remoteService = nullptr;
  remoteService = bleClient->getService(serviceUUID);
  if(remoteService == nullptr)
  {
    Serial.println("-x-Failed to find service UUID!");
    return false;
  }
  Serial.println("-+-Found service!");
  //find characteristic
  music_char = remoteService->getCharacteristic(musicCharUUID);
  if(music_char == nullptr)
  {
    Serial.println("-x-Failed to find music characteristic UUID!");
    return false;
  }
  Serial.println("-+-Found music characteristic!");
  //turn notifications on
  music_char->registerForNotify(notifyMusic);
  music_char->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
  return true;
}

void setup() {
  //restart BLE and flash its memory
  BLEDevice::deinit(true);
  delay(500);
  BLEDevice::init("");

  //scan for servers
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(30);
  while(!serverFound) delay(20);  //wait till server is found

  if(connectToServer(*serverAddress))
  {
    Serial.println("-+-Successfully connected to BLE Server!");
    connected = true;
    serverFound = false;
  }
  else
  {
    Serial.println("-x-Connection failed! Restarting...");
    delay(500);
    ESP.restart();
  }

  rd.attach(13);
  rv.attach(12);
  lv.attach(23);
  ld.attach(22);
  r.attach(15);
  l.attach(21);
  Serial.begin(115200);

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
  while(!go) delay(10);
    timer_reset = millis();
    while ((millis() - timer_reset) < 5000) {
       forward(255);  
       walking();    
       divaPose();   
       delay(300);    
    }
    stop(); 
    delay(2000);

   delay(500);
    left(255);
    delay(11000);
    stop();
    delay(2000);

    unsigned long startTime = millis();
    while (millis() - startTime < 5000) { 
      waving();
    delay(1000);
      waving2();
      delay(1000);
    }
    stop();

    delay(500);
    left(255);
    delay(11000);
    stop();
    delay(2000);
   
    timer_reset = millis();
    while ((millis() - timer_reset) < 4000) {
      waving();    
      delay(1000);  
      waving2(); 
      delay(1000);
    }
    stop();

    delay(1000);

    timer_reset = millis();
    while (millis() - timer_reset < 20000) {  
      askew();
      delay(1000);
      askew2();
      delay(1000);
      askew();
      delay(1000);
      up();
      delay(1500);
      down();
      delay(1500);
      up();
      delay(1500);
      down();
      delay(1500);
      delay(1000);
    }
    stop();
    delay(1000);
    
    /*timer_reset = millis();
    while (millis() - timer_reset < 3000) {  
      horizontal();
      delay(500);
      up();
      delay(500);
      horizontal();
      delay(500);  // <<<< Fixed missing semicolon
      down();
      delay(500);
    }
    stop();

    timer_reset = millis();
    while ((millis() - timer_reset) < 5000) {
      right(255);  
      waving();    
      delay(500);  
      waving2(); 
      delay(500);
    }
    stop();

    timer_reset = millis();
    while ((millis() - timer_reset) < 4000) {
      left(255);  
      waving();    
      delay(500);  
      waving2(); 
      delay(500);
    }
    stop();

    timer_reset = millis();
    while ((millis() - timer_reset) < 11000) {
      to_right_side(255);          
      walking();          
      delay(300);  
      up();
      delay(1000);
      down();
      delay(1000);
    }
    stop();

    delay(500);
    left(255);
    delay(3500);
    stop();

    delay(500);
    right(255);
    delay(3500);
    stop();

    timer_reset = millis();
    while ((millis() - timer_reset) < 4000) {
      askew();
      delay(500);
      askew2();
      delay(500);
    }
    stop();

    timer_reset = millis();
    while ((millis() - timer_reset) < 4000) {          
      up();
      delay(1000);
      down();
      delay(1000);
    }
    stop();

    timer_reset = millis();
    while ((millis() - timer_reset) < 4000) {
      askew();
      delay(500);
      askew2();
      delay(500);
    }
    stop();

    backward(255);
    delay(4000);
    stop();*/

    go = 0;
}




void up() {
  rv.write(0);
  rd.write(0);
  lv.write(10);
  ld.write(30);
}

void down() {
    rv.write(40);
    rd.write(180);
    lv.write(130);
    ld.write(30);
}

void horizontal() {
  rv.write(40);
  rd.write(75);
  lv.write(75);
  ld.write(30);
}

void wave() {
 rd.write(30);
 rv.write(120);
 delay(500);
 lv.write(150);
 ld.write(0);
 delay(500);
 rd.write(150);
 lv.write(30);
 delay(500);
 rv.write(0);
 ld.write(100); 
 delay(500);
}

void divaPose() {
  rv.write(0);
  rd.write(0);
  lv.write(110);
  ld.write(90);
}

void askew() {
  rv.write(50);
  rd.write(130);
  lv.write(30);
  ld.write(30);
}

void askew2() {
  rv.write(50);
  rd.write(30);
  lv.write(100);
  ld.write(40);
}

void waving(){
  rv.write(100);
  rd.write(30);
  lv.write(70);
  ld.write(30);
}

void waving2(){
  rv.write(40);
  rd.write(70);
  lv.write(30);
  ld.write(0);
}

void rlegforward(){
  r.write(20);
}

void llegforward(){
  l.write(160);
}

void rlegbackward(){
  r.write(130);
}

void llegbackward(){
  l.write(50);
}

void rlegstraight(){
  r.write(100);
}

void llegstraight(){
  l.write(85);
}

void walking(){
  r.write(20);
  l.write(50);
  delay(1000);
  l.write(160);
  r.write(130);
  delay(1000);

}

//kolesa

void forward(byte speed)
{
  digitalWrite(RF_DIR[0], LOW);
  digitalWrite(RF_DIR[1], HIGH);
  ledcWrite(RF_EN, speed);

  digitalWrite(LF_DIR[0], LOW);
  digitalWrite(LF_DIR[1], HIGH);
  ledcWrite(LF_EN, speed);

  digitalWrite(RR_DIR[0], LOW);
  digitalWrite(RR_DIR[1], HIGH);
  ledcWrite(RR_EN, speed);

  digitalWrite(LR_DIR[0], LOW);
  digitalWrite(LR_DIR[1], HIGH);
  ledcWrite(LR_EN, speed);
}
void backward(byte speed)
{
  digitalWrite(RF_DIR[0], HIGH);
  digitalWrite(RF_DIR[1], LOW);
  ledcWrite(RF_EN, speed);

  digitalWrite(LF_DIR[0], HIGH);
  digitalWrite(LF_DIR[1], LOW);
  ledcWrite(LF_EN, speed);

  digitalWrite(RR_DIR[0], HIGH);
  digitalWrite(RR_DIR[1], LOW);
  ledcWrite(RR_EN, speed);

  digitalWrite(LR_DIR[0], HIGH);
  digitalWrite(LR_DIR[1], LOW);
  ledcWrite(LR_EN, speed);
}
void left(byte speed)
{
  digitalWrite(RF_DIR[0], LOW);
  digitalWrite(RF_DIR[1], HIGH);
  ledcWrite(RF_EN, speed);

  digitalWrite(LF_DIR[0], HIGH);
  digitalWrite(LF_DIR[1], LOW);
  ledcWrite(LF_EN, speed);

  digitalWrite(RR_DIR[0], LOW);
  digitalWrite(RR_DIR[1], HIGH);
  ledcWrite(RR_EN, speed);

  digitalWrite(LR_DIR[0], HIGH);
  digitalWrite(LR_DIR[1], LOW);
  ledcWrite(LR_EN, speed);
}
void right(byte speed)
{
  digitalWrite(RF_DIR[0], HIGH);
  digitalWrite(RF_DIR[1], LOW);
  ledcWrite(RF_EN, speed);

  digitalWrite(LF_DIR[0], LOW);
  digitalWrite(LF_DIR[1], HIGH);
  ledcWrite(LF_EN, speed);

  digitalWrite(RR_DIR[0], HIGH);
  digitalWrite(RR_DIR[1], LOW);
  ledcWrite(RR_EN, speed);

  digitalWrite(LR_DIR[0], LOW);
  digitalWrite(LR_DIR[1], HIGH);
  ledcWrite(LR_EN, speed);
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
  analogWrite(RF_EN, speed);

  digitalWrite(LF_DIR[0], HIGH);
  digitalWrite(LF_DIR[1], LOW);
  analogWrite(LF_EN, speed);

  digitalWrite(RR_DIR[0], HIGH);
  digitalWrite(RR_DIR[1], LOW);
  analogWrite(RR_EN, speed);

  digitalWrite(LR_DIR[0], LOW);
  digitalWrite(LR_DIR[1], HIGH);
  analogWrite(LR_EN, speed);
}

void to_right_side(int speed) {
  digitalWrite(RF_DIR[0], HIGH);
  digitalWrite(RF_DIR[1], LOW);
  analogWrite(RF_EN, speed);

  digitalWrite(LF_DIR[0], LOW);
  digitalWrite(LF_DIR[1], HIGH);
  analogWrite(LF_EN, speed);

  digitalWrite(RR_DIR[0], LOW);
  digitalWrite(RR_DIR[1], HIGH);
  analogWrite(RR_EN, speed);

  digitalWrite(LR_DIR[0], HIGH);
  digitalWrite(LR_DIR[1], LOW);
  analogWrite(LR_EN, speed);
}