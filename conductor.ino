#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include <ESP32Servo.h>
//---pins---
//ultrasonic
#define TRIG 32
#define ECHO 33
//motors
#define RR_EN 18  // Right rear enable pin
const int RR_DIR[] = {19, 21}; // Right rear direction pins
#define LR_EN 23  // Left rear enable pin
const int LR_DIR[] = {22, 1}; // Left rear direction pins
#define RF_EN 15  // Right front enable pin
const int RF_DIR[] = {2, 4}; // Right front direction pins
#define LF_EN 5   // Left front enable pin
const int LF_DIR[] = {17, 16}; // Left front direction pins
//servos
#define R_ARM 12
#define L_ARM 27
#define R_ELBOW 13
#define L_ELBOW 14


#define SERVER_NAME        "Companero"
#define SERVICE_UUID       "edddc3d6-5d4a-4677-87c5-f4f7d40b6111"  //Service UUID
#define CONTROL_CHAR_UUID  "b4389ca3-0414-43a6-87d2-146c704e8353"  //Characteristic UUID control signals
#define MUSIC_CHAR_UUID    "90c43130-b419-4bfd-bcf4-bb9c8373ddd6"  //Characteristic UUID music sync

Servo right_arm;  //down = 0
Servo left_arm;  //down = 180
Servo right_elbow;  //front = 180
Servo left_elbow;  //front = 0

// Control characteristic and descriptor
BLECharacteristic controlCharacteristic(CONTROL_CHAR_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ);
BLEDescriptor controlDescriptor(BLEUUID((uint16_t)0x2902));
// Music characteristic and descriptor
BLECharacteristic musicCharacteristic(MUSIC_CHAR_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ);
BLEDescriptor musicDescriptor(BLEUUID((uint16_t)0x2902));

// Setup callbacks onConnect and onDisconnect
class MyServerCallbacks: public BLEServerCallbacks
{
  void onConnect(BLEServer* pServer)
  {
    pServer->getAdvertising()->start();
  };
  void onDisconnect(BLEServer* pServer)
  {
    pServer->getAdvertising()->start();
  }
};

//---movement functions---
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

//---functions for arms---
void leftArm(int degrees)
{
  left_arm.write(180-degrees);
  left_elbow.write(90-degrees);
}
void rightArm(int degrees)
{
  right_arm.write(degrees);
  right_elbow.write(degrees+90);
}
//---main code---
void setup()
{
  Serial.begin(115200);

  right_arm.attach(R_ARM);
  left_arm.attach(L_ARM);
  right_elbow.attach(R_ELBOW);
  left_elbow.attach(L_ELBOW);
  leftArm(1);
  rightArm(1);

  for(int i=0; i<2; i++)
  {
    pinMode(RR_DIR[i], OUTPUT);
    pinMode(LR_DIR[i], OUTPUT);
    pinMode(RF_DIR[i], OUTPUT);
    pinMode(LF_DIR[i], OUTPUT);
  }

  ledcAttachChannel(RR_EN, 1000, 8, 0);
  ledcAttachChannel(LR_EN, 1000, 8, 0);
  ledcAttachChannel(RF_EN, 1000, 8, 0);
  ledcAttachChannel(LF_EN, 1000, 8, 0);

  // Create the BLE Device
  BLEDevice::init(SERVER_NAME);

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *robotService = pServer->createService(SERVICE_UUID);

  // Create BLE characteristics and BLE descriptors
  robotService->addCharacteristic(&controlCharacteristic);
  controlCharacteristic.addDescriptor(&controlDescriptor);

  robotService->addCharacteristic(&musicCharacteristic);
  musicCharacteristic.addDescriptor(&musicDescriptor);

  // Start the service
  robotService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pServer->getAdvertising()->start();
}

void loop()
{
  //---go foreard till ultrasonic is close to the curtain---

  //---tell curtains to open---

  //---go to the position---

  //---start music---

  forward(255);
  /*delay(500);
  stop();
  delay(500);
  
  backward(255);
  delay(500);
  stop();
  delay(500);
  
  left(255);
  delay(500);
  stop();
  delay(500);
  
  right(255);
  delay(500);
  stop();
  delay(500);*/
}