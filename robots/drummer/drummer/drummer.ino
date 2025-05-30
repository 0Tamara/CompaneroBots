#include <BLEDevice.h>
#include <ESP32Servo.h>
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

int control_command;
int music_command;

#define R_ARM_PIN 18
#define L_ARM_PIN 21
#define KICK_PIN 19

#define R_UP 80
#define R_DOWN 70
#define L_UP 0
#define L_DOWN 10

Servo r_arm;  //0-80 = down-front
Servo l_arm;  //80-0 = down-front
Servo kick;   //85-90-85 = kick

#define LED_PIN_L 4     //left drum
#define LED_COUNT_L 36
#define LED_PIN_K 5    //kick drum
#define LED_COUNT_K 54
#define LED_PIN_R 17    //right drum
#define LED_COUNT_R 36
#define LED_PIN_EYES 16
#define LED_COUNT_EYES 50

Adafruit_NeoPixel left_ring(LED_COUNT_L, LED_PIN_L, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel kick_ring(LED_COUNT_K, LED_PIN_K, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel right_ring(LED_COUNT_R, LED_PIN_R, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel eyes(LED_COUNT_EYES, LED_PIN_EYES, NEO_GRB + NEO_KHZ800);

unsigned long timer_kick;
unsigned long timer_right;
unsigned long timer_left;
int kicks;
int snares;

byte color_eyes[] = {40, 1, 2}; //pink
byte colors_drums[6][3] = {{255, 0, 0},  //colors that will be cycling over
                           {192, 64, 0},
                           {0, 255, 0},
                           {0, 128, 128},
                           {0, 0, 255},
                           {128, 0, 128}};
int color_index_kick = 0;
int color_index_left = 0;
int color_index_right = 0;

// Zapína LED na všetkých pásikoch naraz
void ledky_vedlajsie() {

  for (int i = 0; i < LED_COUNT_R; i++) {
    right_ring.setPixelColor(i, 255, 0, 0);
    left_ring.setPixelColor(i, 255, 0, 0);

    right_ring.show();
    left_ring.show();
    delay(50);
  }

  for (int i = LED_COUNT_R - 1; i >= 0; i--) {
    right_ring.setPixelColor(i, 0, 0, 0);
    left_ring.setPixelColor(i, 0, 0, 0);

    right_ring.show();
    left_ring.show();
    delay(50);
  }
}
void kick_ring_bubon() {
  for (int i = 0; i < LED_COUNT_K; i++) {
    kick_ring.setPixelColor(i, 255, 0, 0);
    kick_ring.show();
    delay(50);
  }
  for (int i = LED_COUNT_K - 1; i >= 0; i--) {
    kick_ring.setPixelColor(i, 0, 0, 0);
    kick_ring.show();
    delay(50);
  }
}

void changeColorsKick()
{
  if(color_index_kick < 5)
    color_index_kick++;
  else
    color_index_kick = 0;
  for(int i=0; i<LED_COUNT_K; i++)
    kick_ring.setPixelColor(i, colors_drums[color_index_kick][0], colors_drums[color_index_kick][1], colors_drums[color_index_kick][2]);
  kick_ring.show();
}
void changeColorsLeft()
{
  if(color_index_left < 5)
    color_index_left++;
  else
    color_index_left = 0;
  for(int i=0; i<LED_COUNT_L; i++)
    left_ring.setPixelColor(i, colors_drums[color_index_left][0], colors_drums[color_index_left][1], colors_drums[color_index_left][2]);
  left_ring.show();
}
void changeColorsRight()
{
  if(color_index_right < 5)
    color_index_right++;
  else
    color_index_right = 0;
  for(int i=0; i<LED_COUNT_R; i++)
    right_ring.setPixelColor(i, colors_drums[0][color_index_right], colors_drums[1][color_index_right], colors_drums[2][color_index_right]);
  right_ring.show();
}

//---BLE functions---
static void notifyControl(BLERemoteCharacteristic* pBLERemoteControlChar, uint8_t* data, size_t length, bool isNotify)
{
  memcpy(&control_command, data, sizeof(control_command));
  if(control_command == 1)
    openEyes(color_eyes[0], color_eyes[1], color_eyes[2]);
  if(control_command == 2)
  {
    ledky_vedlajsie();
    kick_ring_bubon();
    closeEyes();
    openEyes(color_eyes[0], color_eyes[1], color_eyes[2]);
    closeEyes();
    openEyes(color_eyes[0], color_eyes[1], color_eyes[2]);
  }
}
static void notifyMusic(BLERemoteCharacteristic* pBLERemoteMusicChar, uint8_t* data, size_t length, bool isNotify)
{
  memcpy(&music_command, data, sizeof(music_command));
  if(9 <= music_command && music_command <= 17)
    freedom();
  if(19 <= music_command && music_command <= 32)
    fireball_clapping();
  if(35 <= music_command && music_command <= 41)
    fireball_drop();
  if(43 <= music_command && music_command <= 45)
    fireball_bass();
  if(47 <= music_command && music_command <= 49)
    fireball_bass();
  if(51 <= music_command && music_command <= 64)
    fireball_chill();
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

//---music functions---
void freedom()
{
  timer_kick = 0;
  timer_right = 0;
  kicks = 0;
  snares = 0;
  while(kicks < 4)
  {
    if((millis()-timer_kick) >= 570)
    {
      timer_kick = millis();
      kick.write(90);
      changeColorsKick();
      delay(100);
      kick.write(85);
      kicks ++;
    }
    if(snares < 3)
    {
      if((millis()-timer_right) >= 430)
      {
        timer_right = millis();
        r_arm.write(R_DOWN);
        l_arm.write(L_DOWN);
        changeColorsLeft();
        changeColorsRight();
        delay(100);
        r_arm.write(R_UP);
        l_arm.write(L_UP);
        snares ++;
      }
    }
    else
    {
      if(kicks == 3)
      {
        delay(300);
        r_arm.write(R_DOWN);
        changeColorsRight();
        delay(100);
        r_arm.write(R_UP);
        l_arm.write(L_DOWN);
        changeColorsLeft();
        delay(100);
        l_arm.write(L_UP);
        r_arm.write(R_DOWN);
        changeColorsRight();
        delay(100);
        r_arm.write(R_UP);
      }
    }
  }
}

void fireball_clapping()
{
  timer_right = 0;
  snares = 0;
  while(snares < 4)
  {
    if(!(snares%2))
    {
      if((millis()-timer_right) >= 480)
      {
        timer_right = millis();
        r_arm.write(R_DOWN);
        changeColorsRight();
        delay(100);
        r_arm.write(R_UP);
        snares ++;
      }
    }
    else
    {
      if((millis()-timer_right) >= 480)
      {
        timer_right = millis();
        l_arm.write(L_DOWN);
        changeColorsLeft();
        delay(100);
        l_arm.write(L_UP);
        snares ++;
      }
    }
  }
}

void fireball_drop()
{
  timer_kick = 0;
  kicks = 0;
  while(kicks < 4)
  {
    if((millis()-timer_kick) >= 480)
    {
      timer_kick = millis();
      if(!(kicks%2))
      {
        kick.write(90);
        r_arm.write(R_DOWN);
        l_arm.write(L_DOWN);
        changeColorsKick();
        changeColorsRight();
        changeColorsLeft();
        delay(100);
        kick.write(85);
        r_arm.write(R_UP);
        l_arm.write(L_UP);
      }
      else
      {
        if(kicks == 1)
        {
          kick.write(90);
          r_arm.write(R_DOWN);
          changeColorsKick();
          changeColorsRight();
          delay(100);
          kick.write(85);
          r_arm.write(R_UP);
          while((millis()-timer_kick) < 240) delay(10);
          l_arm.write(L_DOWN);
          changeColorsLeft();
          delay(100);
          l_arm.write(L_UP);
        }
        else
        {
          kick.write(90);
          l_arm.write(L_DOWN);
          changeColorsKick();
          changeColorsLeft();
          delay(100);
          kick.write(85);
          l_arm.write(L_UP);
          while((millis()-timer_kick) < 240) delay(10);
          r_arm.write(R_DOWN);
          changeColorsRight();
          delay(100);
          r_arm.write(R_UP);
        }
      }
      kicks ++;
    }
  }
}

void fireball_bass()
{
  timer_kick = 0;
  kicks = 0;
  while(kicks < 4)
  {
    if((millis()-timer_kick) >= 480)
    {
      timer_kick = millis();
      kick.write(90);
      changeColorsKick();
      delay(100);
      kick.write(85);
      kicks ++;
    }
  }
}


void fireball_chill()
{
  timer_kick = millis();
  kick.write(90);
  r_arm.write(R_DOWN);
  l_arm.write(L_DOWN);
  changeColorsKick();
  changeColorsRight();
  changeColorsLeft();
  delay(100);
  kick.write(85);
  r_arm.write(R_UP);
  l_arm.write(L_UP);
  snares = 0;
  while(snares < 3)
  {
    if(snares%2)
    {
      if((millis()-timer_right) >= 480)
      {
        timer_right = millis();
        r_arm.write(R_DOWN);
        changeColorsRight();
        delay(100);
        r_arm.write(R_UP);
        snares ++;
      }
    }
    else
    {
      if((millis()-timer_right) >= 480)
      {
        timer_right = millis();
        l_arm.write(L_DOWN);
        changeColorsLeft();
        delay(100);
        l_arm.write(L_UP);
        snares ++;
      }
    }
  }
}

//---LEDs functions---
void closeEyes()  //cca 300ms
{
  // Blink LEDs in reverse order (off in sections)
  for (int i = 20; i < 25; i++) eyes.setPixelColor(i, 0, 0, 0); // Left eye
  for (int i = 45; i < 50; i++) eyes.setPixelColor(i, 0, 0, 0); // Right eye
  eyes.show();

  delay(50);

  // Now let's go down the LED sections
  for (int i = 15; i < 20; i++) eyes.setPixelColor(i, 0, 0, 0);
  for (int i = 40; i < 45; i++) eyes.setPixelColor(i, 0, 0, 0);
  eyes.show();

  delay(50);

  for (int i = 10; i < 15; i++) eyes.setPixelColor(i, 0, 0, 0);
  for (int i = 35; i < 40; i++) eyes.setPixelColor(i, 0, 0, 0);
  eyes.show();

  delay(50);

  for (int i = 5; i < 10; i++) eyes.setPixelColor(i, 0, 0, 0);
  for (int i = 30; i < 35; i++) eyes.setPixelColor(i, 0, 0, 0);
  eyes.show();

  delay(50);

  for (int i = 0; i < 5; i++) eyes.setPixelColor(i, 0, 0, 0);
  for (int i = 25; i < 30; i++) eyes.setPixelColor(i, 0, 0, 0);
  eyes.show();

  delay(50);
}

void openEyes(uint8_t red, uint8_t green, uint8_t blue)  //cca 300ms
{
  // Blink LEDs in reverse order (turning LEDs back on)
  for (int i = 0; i < 5; i++) eyes.setPixelColor(i, red, green, blue); // Left eye
  for (int i = 25; i < 30; i++) eyes.setPixelColor(i, red, green, blue); // Right eye
  eyes.show();

  delay(50);

  for (int i = 5; i < 10; i++) eyes.setPixelColor(i, red, green, blue);
  for (int i = 30; i < 35; i++) eyes.setPixelColor(i, red, green, blue);
  eyes.show();

  delay(50);

  for (int i = 10; i < 15; i++) eyes.setPixelColor(i, red, green, blue);
  for (int i = 35; i < 40; i++) eyes.setPixelColor(i, red, green, blue);
  eyes.show();

  delay(50);

  for (int i = 15; i < 20; i++) eyes.setPixelColor(i, red, green, blue);
  for (int i = 40; i < 45; i++) eyes.setPixelColor(i, red, green, blue);
  eyes.show();

  delay(50);

  for (int i = 20; i < 25; i++) eyes.setPixelColor(i, red, green, blue);
  for (int i = 45; i < 50; i++) eyes.setPixelColor(i, red, green, blue);
  eyes.show();

  delay(50);
}

//---main code---
void setup()
{
  Serial.begin(115200);

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

  r_arm.attach(R_ARM_PIN);
  l_arm.attach(L_ARM_PIN);
  kick.attach(KICK_PIN);
  
  kick.write(85);
  r_arm.write(R_UP);
  l_arm.write(L_UP);

  left_ring.begin();
  left_ring.show();
  kick_ring.begin();
  kick_ring.show();
  right_ring.begin();
  right_ring.show();
  eyes.begin();
  eyes.show();
}

void loop()
{
}
