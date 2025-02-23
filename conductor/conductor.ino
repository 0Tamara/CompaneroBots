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

//---constants---
#define SERVER_NAME        "Companero"  //BLE server
#define SERVICE_UUID       "edddc3d6-5d4a-4677-87c5-f4f7d40b6111"  //service UUID
#define CONTROL_CHAR_UUID  "b4389ca3-0414-43a6-87d2-146c704e8353"  //characteristic UUID performance control
#define MUSIC_CHAR_UUID    "90c43130-b419-4bfd-bcf4-bb9c8373ddd6"  //characteristic UUID music sync

//control characteristic and descriptor
BLECharacteristic controlCharacteristic(CONTROL_CHAR_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ);
BLEDescriptor controlDescriptor(BLEUUID((uint16_t)0x2902));
//music characteristic and descriptor
BLECharacteristic musicCharacteristic(MUSIC_CHAR_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ);
BLEDescriptor musicDescriptor(BLEUUID((uint16_t)0x2902));

Servo right_arm;  //down = 0
Servo left_arm;  //down = 180
Servo right_elbow;  //front = 180
Servo left_elbow;  //front = 0

//---variables---
int progress = 0;  //when in the preformance we are

unsigned long pulse_time;  //time for sound to return to ultrasonic

unsigned long timer;  //millis

//setup callbacks onConnect and onDisconnect
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

float distance()  //measure distance in cm
{
  float distance;
  digitalWrite(TRIG, LOW);  //clear TRIG
  delayMicroseconds(2);

  digitalWrite(TRIG, HIGH);  //send sound for 10us
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  pulse_time = pulseIn(ECHO, HIGH);  //time from sending sound till it gets back

  distance = pulse_time * 0.017;  //speed of sound[cm/us] / 2 = 0.017
  return distance;
}
//---main code---
void setup()
{
  Serial.begin(115200);

  //---BLE---
  BLEDevice::init(SERVER_NAME);  //create the BLE Device
  BLEServer *pServer = BLEDevice::createServer();  //create the BLE Server
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *robotService = pServer->createService(SERVICE_UUID);  //create the BLE Service
  //create BLE characteristics and BLE descriptors
  robotService->addCharacteristic(&controlCharacteristic);
  controlCharacteristic.addDescriptor(&controlDescriptor);
  robotService->addCharacteristic(&musicCharacteristic);
  musicCharacteristic.addDescriptor(&musicDescriptor);

  robotService->start();  //start the service
  //start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pServer->getAdvertising()->start();

  //---pins---
  //servos
  right_arm.attach(R_ARM);
  left_arm.attach(L_ARM);
  right_elbow.attach(R_ELBOW);
  left_elbow.attach(L_ELBOW);
  leftArm(1);
  rightArm(1);
  //motors
  for(int i=0; i<2; i++)
  {
    pinMode(RR_DIR[i], OUTPUT);
    pinMode(LR_DIR[i], OUTPUT);
    pinMode(RF_DIR[i], OUTPUT);
    pinMode(LF_DIR[i], OUTPUT);
  }
  ledcAttachChannel(RR_EN, 1000, 8, 1);
  ledcAttachChannel(LR_EN, 1000, 8, 1);
  ledcAttachChannel(RF_EN, 1000, 8, 1);
  ledcAttachChannel(LF_EN, 1000, 8, 1);
  //ultrasonic
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
}

void loop()
{
  //---start the performance with holding a hand on ultrasonic for 1sec---
  /*if(progress == 0)
  {
    if(distance() < 10)
    {
      timer = millis();
      while(distance() < 10)
      {
        if((millis() - timer) > 1000)
        {
          progress = 1;
          break;
        }
      }
    }
  }*/
  //---go foreard till ultrasonic is close to the curtain---

  //---tell curtains to open---

  //---go to the position---

  //---start music---
  //---freedom---
  progress = 1;
  musicCharacteristic.setValue(progress);
  while(progress < 17)
  {
    if((millis()-timer) >= 2280)
    {
      timer = millis();
      musicCharacteristic.notify();
      progress ++;
      musicCharacteristic.setValue(progress);
    }
  }

  delay(10000);
}