#include <BLEDevice.h>
#include <Adafruit_NeoPixel.h>

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

bool control = 1;

#define LED_PIN_KEYS 25  //led strip behind the keys
#define LED_COUNT_KEYS 21
#define LED_PIN_EYES 26
#define LED_COUNT_EYES 50
#define LED_PIN_LEGO 27  //led for communication with lego
#define LED_COUNT_LEGO 1

Adafruit_NeoPixel keys(LED_COUNT_KEYS, LED_PIN_KEYS, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel eyes(LED_COUNT_EYES, LED_PIN_EYES, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel lego(LED_COUNT_LEGO, LED_PIN_LEGO, NEO_GRB + NEO_KHZ800);

int control_command;
int music_command;

// Funkcia pre efekt "Night Rider"
void knightRiderEffect() {
  int mid = LED_COUNT / 2;  // Nájdeme strednú LED
  
  // Prvá fáza: od stredu smerom von
  for (int step = 0; step <= mid; step++) {
    strip.clear();
    
    if (mid - step >= 0) strip.setPixelColor(mid - step, 255, 0, 0); // Červená
    if (mid + step < LED_COUNT) strip.setPixelColor(mid + step, 255, 0, 0);

    strip.show();  // Aktualizujeme LEDky až po nastavení všetkých
    delay(45);
  }

  // Druhá fáza: z okrajov späť do stredu
  for (int step = mid; step >= 0; step--) {
    strip.clear();
    
    if (mid - step >= 0) strip.setPixelColor(mid - step, 255, 0, 0);
    if (mid + step < LED_COUNT) strip.setPixelColor(mid + step, 255, 0, 0);

    strip.show();  // Aktualizujeme LEDky až po nastavení všetkých
    delay(45);
  }
}

//---functions for getting notifications---
static void notifyControl(BLERemoteCharacteristic* pBLERemoteControlChar, uint8_t* data, size_t length, bool isNotify)
{
  memcpy(&control_command, data, sizeof(control_command));
  if(control_command == 3)
  {
    for(int i=0; i<4; i++)
      knightRiderEffect();
  }

  if(control_command == 4)
  {
    lego.setPixelColor(0, 255, 255, 255);
    lego.show();
    delay(1000);
    lego.setPixelColor(0, 0, 0, 0);
    lego.show();
  }
}
static void notifyMusic(BLERemoteCharacteristic* pBLERemoteMusicChar, uint8_t* data, size_t length, bool isNotify)
{
  memcpy(&music_command, data, sizeof(music_command));
  if(music_command == 19)
  {
    lego.setPixelColor(0, 255, 255, 255);
    lego.show();
    knightRiderEffect();
    lego.setPixelColor(0, 0, 0, 0);
    lego.show();
  }
  else
    knightRiderEffect();
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

  keys.begin();
  keys.show();
  eyes.begin();
  eyes.show();
  lego.begin();
  lego.show();
}

void loop()
{
}
