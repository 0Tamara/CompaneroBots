#include "BLEDevice.h"

#define bleServerName "Companero"  //name of the BLE server we are connecting to
//UUIDs of the service and characteristics:
static BLEUUID serviceUUID("edddc3d6-5d4a-4677-87c5-f4f7d40b6111");
static BLEUUID controlCharUUID("b4389ca3-0414-43a6-87d2-146c704e8353");
static BLEUUID musicCharUUID("90c43130-b419-4bfd-bcf4-bb9c8373ddd6");

static BLEAddress* serverAddress;  //address of the device
static BLERemoteCharacteristic* control_char;  //characteristic for preformance control
static BLERemoteCharacteristic* music_char;  //characteristic for music sync
//notification options
const uint8_t notificationOn[] = {0x1, 0x0};
const uint8_t notificationOff[] = {0x0, 0x0};

static bool serverFound = false;
static bool connected = false;

int control_command;
int music_command;

//---functions for getting notifications---
static void notifyControl(BLERemoteCharacteristic* pBLERemoteControlChar, uint8_t* data, size_t length, bool isNotify)
{
  memcpy(&control_command, data, sizeof(control_command));
  Serial.print("Control command: ");
  Serial.println(control_command);
  digitalWrite(2, HIGH);
}
static void notifyMusic(BLERemoteCharacteristic* pBLERemoteMusicChar, uint8_t* data, size_t length, bool isNotify)
{
  memcpy(&music_command, data, sizeof(music_command));
  Serial.print("Music command: ");
  Serial.println(music_command);
  digitalWrite(2, LOW);
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
  //find characteristics
  control_char = remoteService->getCharacteristic(controlCharUUID);
  if(control_char == nullptr)
  {
    Serial.println("-x-ailed to find control characteristic UUID!");
    return false;
  }
  Serial.println("-+-Found control characteristic!");
  music_char = remoteService->getCharacteristic(musicCharUUID);
  if(music_char == nullptr)
  {
    Serial.println("-x-Failed to find music characteristic UUID!");
    return false;
  }
  Serial.println("-+-Found music characteristic!");
  //turn notifications on
  control_char->registerForNotify(notifyControl);
  control_char->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
  music_char->registerForNotify(notifyMusic);
  music_char->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
  return true;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");

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

  pinMode(2, OUTPUT);
}

void loop()
{
}
