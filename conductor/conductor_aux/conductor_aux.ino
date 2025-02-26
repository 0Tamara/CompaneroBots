#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

//---constants---
#define SERVER_NAME   "Companero_aux"  //BLE server
#define SERVICE_UUID  "89cf8ec9-04a9-4814-93be-f092e7a5272f"  //service UUID
#define AUX_CHAR_UUID "76745552-3961-4938-9f0a-09a6181dc352"  //auxiliary characteristic UUID

//control characteristic and descriptor
BLECharacteristic auxCharacteristic(AUX_CHAR_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ);
BLEDescriptor auxDescriptor(BLEUUID((uint16_t)0x2902));

//---variables---
int message;

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
  robotService->addCharacteristic(&auxCharacteristic);
  auxCharacteristic.addDescriptor(&auxDescriptor);

  robotService->start();  //start the service
  //start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pServer->getAdvertising()->start();
}

void loop()
{
  if(Serial.available())
  {
    message = Serial.read()-0x30;
    auxCharacteristic.setValue(message);
    auxCharacteristic.notify();
  }
}