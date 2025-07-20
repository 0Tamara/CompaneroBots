#include <esp_log.h>
#include <FastAccelStepper.h>
#include <Adafruit_PWMServoDriver.h>
#include <Wire.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>

#define SERVOMIN  125
#define SERVOMAX  575
#define numServos 16
#define stepsPerNote 929
#define stepsPerOctave 6498
#define speedInHz 15000
#define acceleration 40000

const int leftHandStepPin = 5; 
const int leftHandDirPin = 16; 
const int leftHandEnPin = 18;
const int rightHandStepPin = 4;
const int rightHandDirPin = 17;
const int rightHandEnPin = 15;
// casy
typedef struct struct_message {
  byte song; //time during being on/off
  byte time; //ms
} struct_message;

typedef struct send_message {
  byte end;
} send_message;

esp_now_peer_info_t peerInfo;

// Create a struct_message called myData
uint8_t camAddr[] = {0xC0, 0x49, 0xEF, 0xD0, 0x8C, 0xC0};
struct_message myData;
send_message sendData;
int targetNoteRight = 0;
int targetOctaveRight = 0;
int targetNoteLeft = 0;
int targetOctaveLeft = 0;
int stepsRight = 0;
int stepsLeft = 0;
int timeFromMoving = 0;
int tempo = 2280;
int sest = tempo / 16; 
unsigned long start = millis();
const int rezerva = 20;

// kniznice
Adafruit_PWMServoDriver pca9685right(0x41, Wire);
Adafruit_PWMServoDriver pca9685left(0x40, Wire);

FastAccelStepperEngine engine = FastAccelStepperEngine();

enum moveNotes { C=0, D=1, E=2, F=3, G=4, A=5, H=6 };

struct Hand
{
  int currentOctave;
  int currentNote;
  FastAccelStepper* stepper;
  unsigned long timeFromMoving;
  unsigned long lastTime;
  int pressValue;
  int releaseValue;
};

Hand leftHand = {
  .currentOctave = 0,
  .currentNote = 0,
  .stepper = NULL,
  .timeFromMoving = 0,
  .lastTime = 0,
  .pressValue = SERVOMAX - 100,
  .releaseValue = SERVOMAX,
};
Hand rightHand = {
  .currentOctave = 0,
  .currentNote = 0,
  .stepper = NULL,
  .timeFromMoving = 0,
  .lastTime = 0,
  .pressValue = SERVOMIN + 100,
  .releaseValue = SERVOMIN,
};

int finalCountdownRightPosition1[] = {F, 1};
int finalCountdownRight1[] =
{
  0b00001000,//stv pomcka
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,//osm pomlcka
  0b00000000,
  0b00000010,//sest 
  0b00000100,//sest
  0b00000010,//stv
  0b00000000,
  0b00000000,
  0b00000000,
  0b00100000,//stv
  0b00000000,
  0b00000000,
  0b00000000,
}; 
int finalCountdownLeftPosition1[] = {A, 2};
int finalCountdownLeft1[] =
{
  0b10000001,//stv pomcka
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,//stv pomlcka
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,//stv pomlcka
  0b00000000,
  0b00000000, 
  0b00000000,
  0b00000001,//osm
  0b00000000,
  0b00000010, 
  0b00000000,
};
int finalCountdownRight2[] =
{
  0b00000000,//stv pomcka
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,//osm pomlcka
  0b00000000,
  0b00000001,//sest 
  0b00000010,//sest
  0b00000001,//osm
  0b00000000,
  0b00000010, //osm
  0b00000000,
  0b00000100,//stv
  0b00000000,
  0b00000000,
  0b00000000,
};
int finalCountdownLeft2[] =
{
  0b00000100,//stv pomcka
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,//stv pomlcka
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,//stv pomlcka
  0b00000000,
  0b00000000, 
  0b00000000,
  0b00000100,//osm
  0b00000000,
  0b00001000, //osm
  0b00000000,
};
int finalCountdownRight3[] =
{
  0b00000000,//stv pomcka
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,//osm pomlcka
  0b00000000,
  0b00000001,//sest 
  0b00000010,//sest
  0b00000001,//stv
  0b00000000,
  0b00000000, 
  0b00100000,
  0b00000000,//stv
  0b00000000,
  0b00000000,
  0b00000000,
};
int finalCountdownLeft3[] =
{
  0b00010000,//stv
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000, 
  0b00000000,
  0b00010000,//stv
  0b00000000,
  0b00000000, 
  0b00000000,
};
int finalCountdownRight4[] =
{
  0b01000000,//stv 
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,//osm pomlcka
  0b00000000,
  0b00000100,//sest 
  0b00001000,//sest
  0b00000100,//osm
  0b00000000,
  0b00001000, //osm
  0b00000000,
  0b00010000,//osm
  0b00000000,
  0b00000100,//osm
  0b00000000,
};
int finalCountdownLeft4[] = 
{
  0b00000010,//stv pomcka
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,//stv pomlcka
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,//stv pomlcka
  0b00000000,
  0b00000000, 
  0b00000000,
  0b00000000,//stv
  0b00000000,
  0b00000000, 
  0b00000000,
};
int kernkraftRightPosition1[] = {F, 2};
int kernkraftRight1[] = 
{
  0b00000000,//osm pomlcka
  0b00000000,
  0b00100000,//osm
  0b00000000,
  0b00001000,//osm
  0b00000000,
  0b00000100,//osm
  0b00000000,
  0b00000010,//osm
  0b00000000,
  0b00100000,//osm
  0b00000000,
  0b00000000,//stv pomlcka
  0b00000000,
  0b00000000,
  0b00000000,
};

int kernkraftLeftPosition1[] = {A, 1};
int kernkraftLeftPosition2[] = {D, 2};
int kernkraftLeft1[]
{
  0b10000000, //osm
  0b00000000,
  0b00000001, //osm
  0b00000000,
  0b10000000, //osm
  0b00000000,
  0b00000001, //osm
  0b00000000,
  0b10000000, //osm
  0b00000000,
  0b00000001, //osm
  0b00000000,
  0b10000000, //osm
  0b00000000,
  0b00000001, //osm
  0b00000000,
};
int kernkraftRight2[] =
{
  0b00000000,//osm pomlcka
  0b00000000,
  0b00100000,//osm
  0b00000000,
  0b00001000,//osm
  0b00000000,
  0b00000100,//osm
  0b00000000,
  0b00000010,//osm
  0b00000000,
  0b00000001,//osm
  0b00000000,
  0b00000010,//osm
  0b00000000,
  0b00001000,//osm
  0b00000000,
};
int kernkraftRight3[]
{
  0b00000100,//osm
  0b00000000,
  0b00000000,//stv
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000100,//osm
  0b00000000,
  0b00001000,//osm
  0b00000000,
  0b00000000,//osm pomlcka
  0b00000000,
  0b00000010,//osm
  0b00000000,
  0b00100000,//osm
  0b00000000,
};
int kernkraftRight4[]
{
  0b00000000,//cel
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
};
int barLeft[16];
int barRight[16];
int positionLeft[2];
int positionRight[2];
//opatrujte niekto tento kod, je to posledny kod ktory napisem na robocup onstage pre klaviristu
void playBar(){ 
  targetNoteRight = positionRight[0];
  targetOctaveRight = positionRight[1];
  targetNoteLeft = positionLeft[0];
  targetOctaveLeft = positionLeft[1];
  stepsRight = stepsPerNote * targetNoteRight + stepsPerNote * (targetOctaveRight - 1);
  stepsLeft = stepsPerNote * targetNoteLeft + stepsPerNote * (targetOctaveLeft - 1);
  rightHand.stepper->moveTo(stepsRight);
  leftHand.stepper->moveTo(stepsLeft);
  for (int i = 0; i < 16; i++)
  {
    int wait = sest;
    byte notesRight = barRight[i];
    byte notesLeft = barLeft[i];
    Serial.printf("notes number %d\n", i);
    
    for (int j = 0; j < 8; j++)
    {
      if ((notesRight & 1<<j) && !(rightHand.stepper->isRunning()))
      {
        pca9685right.setPWM(j+8, 0, rightHand.pressValue);
      }
      if ((notesLeft & 1<<j) && !(leftHand.stepper->isRunning()))
      {
        pca9685left.setPWM(j+8, 0, leftHand.pressValue);
      }
    }
    
    delay(75);
    for (int j = 0; j < 8; j++)
    {
      pca9685right.setPWM(j+8, 0, rightHand.releaseValue);
      pca9685left.setPWM(j+8, 0, leftHand.releaseValue);
    }
    while (millis() - start <= wait * i) 
    {
    }
  }
  Serial.println();
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  start = millis();
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.printf("song = %d\n", myData.song);
  Serial.printf("time = %d\n", myData.time);
  if(myData.song == 1)
  {
    rightHand.stepper->moveTo(0);
    while (rightHand.stepper->isRunning()) {
    } 
    leftHand.stepper->moveTo(0);
    while (leftHand.stepper->isRunning()) {
    }
  }

  if(myData.song == 2)
  { 
    //prvy takt
    start = millis();
    tempo = 2208; 
    sest = tempo / 16; 
    for(int i=0; i<16; i++)
    {
      barLeft[i] = finalCountdownLeft1[i];
      barRight[i] = finalCountdownRight1[i];
    }
    for(int i=0; i<2; i++)
    {
      positionLeft[i] = finalCountdownLeftPosition1[i];
      positionRight[i] = finalCountdownRightPosition1[i];
    }
    playBar();
    while(millis() - start <= tempo ){}
    //druhy takt
    start = millis();
    for(int i=0; i<16; i++)
    {
      barLeft[i] = finalCountdownLeft2[i];
      barRight[i] =finalCountdownRight2[i];
    }
    playBar();
    while(millis() - start <= tempo ){}
    //treti takt
    start = millis();
    for(int i=0; i<16; i++)
    {
      barLeft[i] = finalCountdownLeft3[i];
      barRight[i] =finalCountdownRight3[i];
    }
    playBar();
    while(millis() - start <= tempo ){}
    //stvrty takt
    start = millis();
    for(int i=0; i<16; i++)
    {
      barLeft[i] = finalCountdownLeft4[i];
      barRight[i] =finalCountdownRight4[i];
    }
    playBar();
    while(millis() - start <= tempo ){}
    //odoslanie
    sendData.end = 1;
    esp_now_send(camAddr, (uint8_t *) &sendData, sizeof(sendData));
    Serial.printf("Data sended: %d\n",sendData.end );
  }
  if(myData.song == 3)
  {
    //prvy takt
    start = millis();
    tempo = 1950;
    sest = tempo / 16;
    rightHand.stepper->moveTo(stepsPerNote * kernkraftRightPosition1[0] + stepsPerOctave * kernkraftRightPosition1[1]);
    while (rightHand.stepper->isRunning()) {
    } 
    leftHand.stepper->moveTo(stepsPerNote * kernkraftLeftPosition1[0] + stepsPerOctave * kernkraftLeftPosition1[1]);
    while (leftHand.stepper->isRunning()) {
    }
    for(int i=0; i<16; i++)
    {
      barLeft[i] = kernkraftLeft1[i];
      barRight[i] = kernkraftRight1[i];
    }
    for(int i=0; i<2; i++)
    {
      positionLeft[i] = kernkraftRightPosition1[i];
      positionRight[i] = kernkraftLeftPosition1[i];
    }
    playBar();
    while(millis() - start <= tempo){}
    //druhy takt
    start = millis();
    for(int i=0; i<16; i++)
    {
      barLeft[i] = kernkraftLeft1[i];
      barRight[i] = kernkraftRight2[i];
    }
    playBar();
    while(millis() - start <= tempo){}
    //treti takt
    start = millis();
    for(int i=0; i<16; i++)
    {
      barLeft[i] = kernkraftLeft1[i];
      barRight[i] = kernkraftRight3[i];
    }
    for(int i=0; i<2; i++)
    {
      positionLeft[i] = kernkraftRightPosition1[i];
      positionRight[i] = kernkraftLeftPosition2[i];
    }
    playBar();
    while(millis() - start <= tempo){}
    //stvrty takt 
    start = millis();
    for(int i=0; i<16; i++)
    {
      barLeft[i] = kernkraftLeft1[i];
      barRight[i] = kernkraftRight4[i];
    }
    for(int i=0; i<2; i++)
    {
      positionLeft[i] = kernkraftRightPosition1[i];
      positionRight[i] = kernkraftLeftPosition1[i];
    }
    playBar();
    while(millis() - start <= tempo){}

    sendData.end = 1;
    esp_now_send(camAddr, (uint8_t *) &sendData, sizeof(sendData));
    Serial.printf("Data sended: %d\n",sendData.end );
  }
}

void setup() {
  Serial.begin(115200);
  esp_log_level_set("i2c.master", ESP_LOG_NONE);
  Wire.begin(21, 22);
  pca9685right.begin();
  pca9685left.begin();
  pca9685right.setPWMFreq(50);
  pca9685left.setPWMFreq(50); 
  for (int i = 8; i <= numServos; i++){
    pca9685right.setPWM(i, 0, rightHand.releaseValue);
    pca9685left.setPWM(i, 0, leftHand.releaseValue); // 0 stupnov
  }
  engine.init();
  leftHand.stepper = engine.stepperConnectToPin(leftHandStepPin);
  rightHand.stepper = engine.stepperConnectToPin(rightHandStepPin);
  if (rightHand.stepper == NULL || leftHand.stepper == NULL) {
    Serial.println("Chyba pri pripojeni krokovych motorov.");
    while (1);
  }

  //init WiFi & read MAC address
  WiFi.mode(WIFI_STA);
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  Serial.printf("My MAC address: {0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X}\n",
                baseMac[0], baseMac[1], baseMac[2],
                baseMac[3], baseMac[4], baseMac[5]);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Register peer
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  
  // Add peer
  memcpy(peerInfo.peer_addr, camAddr, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  
  // register recv callback
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
  
  leftHand.stepper->setDirectionPin(leftHandDirPin);
  leftHand.stepper->setEnablePin(leftHandEnPin);
  leftHand.stepper->setAutoEnable(true); 

  leftHand.stepper->setSpeedInHz(speedInHz);
  leftHand.stepper->setAcceleration(acceleration);
  leftHand.stepper->setCurrentPosition(0);
  
  rightHand.stepper->setDirectionPin(rightHandDirPin);
  rightHand.stepper->setEnablePin(rightHandEnPin);
  rightHand.stepper->setAutoEnable(true);

  rightHand.stepper->setSpeedInHz(speedInHz);
  rightHand.stepper->setAcceleration(acceleration);
  rightHand.stepper->setCurrentPosition((stepsPerOctave * 2));  
}
 
void loop() {
}