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
unsigned long timeBeforeMoving = 0;
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

int havasiFreedomRightPosition1[]{A, 1};
int havasiFreedomLeftPosition1[]{A, 1};
int havasiFreedomRight1[]{
  0b00000000, //cel pomlcka
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

int havasiFreedomLeft1[] = {
  0b10000000, //osm
  0b00000000,
  0b00000000, // sest pomlcka
  0b00000001, //osm
  0b00000000, 
  0b00000000, //sest pomlcka
  0b10000001, //osm
  0b00000000,
  0b10000001, //stv
  0b00000000,
  0b00000000,
  0b00000000, 
  0b10000001, //stv
  0b00000000,
  0b00000000,
  0b00000000,
};
int havasiFreedomRight5[] = {
  0b10101000, // osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b10101000, // osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b10100000, // stv
  0b00000000,
  0b00000000,
  0b00000000,
  0b10000000, //sest
  0b01000000, //sest
  0b00100000, //sest
  0b01010000, //osm
  0b00000000, 
  0b00001000, //sest
}; 
int havasiFreedomLeft5[] = {
  0b10000001, //osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b10000001, //osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b10000001, //osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b10000001, //osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b10000001, //stv
  0b00000000,
  0b00000000, 
  0b00000000,
};
int havasiFreedomRight6[] = {
  0b10100000, //stv
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000, // osm pomlcka
  0b00000000,
  0b00000000, //stvrtova pomlcka
  0b00000000,
  0b00000000,
  0b00000000,
  0b01000000, //sest
  0b00100000, //sest
  0b01010000, //osm
  0b00000000,
  0b00101000, //sest
  0b00000000,
};
int havasiFreedomRight7[] = {
  //este len zacatok a uz si uvedomujem jak krasne vonka svieti slnko
  0b10101000, // osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b10100000, // osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b00000000, // stv pomlcka
  0b00000000,
  0b00000000,
  0b00000000,
  0b10000000, //sest
  0b01000000, //sest
  0b10100000, //osm
  0b00000000,
  0b00000000, // sest pomlcka
  0b01010000, // sest
  
};
int havasiFreedomRight8[] = {
  0b10100000, //stv
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000, //osm pomlcka
  0b00000000,
  0b00000000, //stv pomlcka
  0b00000000,
  0b00000000,
  0b00000000,
  0b10000000, //sest
  0b01000000, //sest
  0b10100000, //osm
  0b00000000,
  0b10010000, //osm
  0b00000000,
};

int havasiFreedomLeftPosition9[] = {F, 1};
int havasiFreedomRight9[] = {
  //uz len kuuus... 
  0b10101000, // osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b10101000, // osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b00000000, // stv
  0b00000000,
  0b00000000,
  0b00000000,
  0b10000000, //sest
  0b01000000, //sest
  0b10100000, //osm
  0b00000000,
  0b00000000, //sest pomlcka 
  0b01010000, //sest
};
int havasiFreedomRight10[] = {
  0b10100000, //stv
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000, //osm pomlcka
  0b00000000,
  0b00000000, //stv pomlcka
  0b00000000,
  0b00000000,
  0b00000000,
  0b10000000, //sest
  0b01000000, //sest
  0b10100000, //osm
  0b00000000,
  0b01010000, //osm
  0b00000000,
};
int havasiFreedomRight11[] = {
  0b10101000, // osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b10100000, // osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b00000000, // stv pomlcka
  0b00000000,
  0b00000000,
  0b00000000,
  0b10000000, //sest
  0b01000000, //sest
  0b10100000, //osm
  0b00000000,
  0b01010000, // osm
  0b00000000, 
};
int havasiFreedomRightPosition12[] = {G, 1};
int havasiFreedomLeftPosition12[] = {E, 1};
int havasiFreedomRight12[] = {
  //jak to bolo? Tech tisic mil tech tisic mil ma jeden daco, a jeden cil
  0b00100010, //osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b00100010, //osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b00100010, //osm
  0b00000000,
  0b01000010, //stv
  0b00000000, 
  0b00000000, 
  0b00000000, 
  0b01000010, //stv
  0b00000000, 
  0b00000000, 
  0b00000000, 
};
int havasiFreedomLeft12[] = {
  0b10000001, //osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b10000001, //osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b10000001, //osm
  0b00000000,
  0b10000001, //stv
  0b00000000, 
  0b00000000, 
  0b00000000, 
  0b10000001, //stv
  0b00000000, 
  0b00000000, 
  0b00000000,   
};
int fireballRightPosition1[] = {G, 1};
int fireballRight1[] = {
  0b10000000, //stv
  0b00000000, 
  0b00000000, 
  0b00000000,  
  0b00000001, //osm
  0b00000000, 
  0b00000001, //osm
  0b00000000, 
  0b00000010, //sest
  0b00000010, //osm
  0b00000000, 
  0b00000010, //sest
  0b00000000, //stv pomlcka
  0b00000000, 
  0b00000000, 
  0b00000000, 
};
int fireballLeftPosition1[] = {G, 1};
int fireballLeft1[] = {
  0b10000000, //stv
  0b00000000, 
  0b00000000, 
  0b00000000,  
  0b00001000, //osm
  0b00000000,  
  0b00000010, //osm
  0b00000000,  
  0b00000001, //stv
  0b00000000, 
  0b00000000, 
  0b00000000,  
  0b00000010, //osm
  0b00000000,  
  0b00001000, //osm
  0b00000000,  
};
int fireballRight2[] = {
  0b00000000, //osm pomlcka
  0b00000000, 
  0b10000000, //osm nota
  0b00000000,  
  0b00000001, //osm
  0b00000000, 
  0b00000001, //osm
  0b00000000, 
  0b00000010, //sest
  0b00000010, //osm
  0b00000000, 
  0b00000010, //sest
  0b00000000, //stv pomlcka
  0b00000000, 
  0b00000000, 
  0b00000000, 
};
int fireballRightPosition5[]{D, 2};
int fireballRight5[] = {
  0b00010000, //stv
  0b00000000, 
  0b00000000, 
  0b00000000,  
  0b00001000, //osm
  0b00000000, 
  0b00100000, //osm
  0b00000000, 
  0b00010000, //sest
  0b00000000, //osm
  0b00000000, 
  0b10000000, //sest
  0b00000000, //stv pomlcka
  0b00000000, 
  0b00000000, 
  0b00000000, 
};
int fireballRight6[] = {
  0b00000000, //osm pomlcka
  0b00000000, 
  0b00010000, //osm 
  0b00000000,  
  0b00001000, //osm
  0b00000000, 
  0b00100000, //osm
  0b00000000, 
  0b00010000, //sest
  0b00000000, //osm
  0b00000000, 
  0b10000000, //sest
  0b00000000, //stv pomlcka
  0b00000000, 
  0b00000000, 
  0b00000000, 
};
int fireballRight8[] = {
  0b00000000, //osm pomlcka
  0b00000000, 
  0b00010000, //osm 
  0b00000000,  
  0b00001000, //osm
  0b00000000, 
  0b00100000, //osm
  0b00000000, 
  0b00010000, //sest
  0b00000000, //osm
  0b00000000, 
  0b00000001, //sest
  0b00000000, //stv pomlcka
  0b00000000, 
  0b00000000, 
  0b00000000, 
};
int stupnicaRightAndLeftPosition[]{C, 1};
int stupnicaRightAndLeft[]{
  0b10000000,
  0b01000000,
  0b00100000,
  0b00010000,
  0b00001000,
  0b00000100,
  0b00000010,
  0b00000001,
  0b00000001,
  0b00000010,
  0b00000100,
  0b00001000,
  0b00010000,
  0b00100000,
  0b01000000,
  0b10000000,
};
int osudovaRightAndLeftPosition[]{C, 1};
int osudovaRightAndLeft1[]{
  0b00000000,
  0b00000000,
  0b00000000, //osm pomlcka
  0b00000000,
  0b00010000,
  0b00000000,
  0b00000000, //osm 
  0b00000000,
  0b00010000,
  0b00000000,
  0b00000000, //osm
  0b00000000,
  0b00010000,
  0b00000000,
  0b00000000, // osm
  0b00000000,
};
int osudovaRightAndLeft2[]{
  0b01000000, // osm
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
int osudovaRightAndLeft3[]{
  
  0b00000000, //osm pomlcka
  0b00000000,
  0b00000000,
  0b00000000,
  0b00100000, //osm 
  0b00000000,
  0b00000000,
  0b00000000,
  0b00100000, 
  0b00000000,
  0b00000000,//osm
  0b00000000,
  0b00100000, // osm
  0b00000000,
  0b00000000,
  0b00000000,
};
int osudovaRightAndLeft4[]{
  0b10000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000, // osm
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

void playBar(){ 
  targetNoteRight = positionRight[0];
  targetOctaveRight = positionRight[1];
  targetNoteLeft = positionLeft[0];
  targetOctaveLeft = positionLeft[1];
  stepsRight = stepsPerNote * targetNoteRight + stepsPerNote * (targetOctaveRight - 1);
  stepsLeft = stepsPerNote * targetNoteLeft + stepsPerNote * (targetOctaveLeft - 1);
  rightHand.stepper->moveTo(stepsRight);
  leftHand.stepper->moveTo(stepsLeft);
  timeBeforeMoving = millis();
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
    while (leftHand .stepper->isRunning()) {
    }
  }
  if(myData.song == 2)
  {
    tempo = 890; 
    sest = tempo / 16; 
    for(int i=0; i<16; i++)
    {
      barLeft[i] = osudovaRightAndLeft1[i];
      barRight[i] = osudovaRightAndLeft1[i];
    }
    for(int i=0; i<2; i++)
    {
      positionLeft[i] = osudovaRightAndLeftPosition[i];
      positionRight[i] = osudovaRightAndLeftPosition[i];
    }
    playBar();
    while(millis() - start <= tempo ){}
    //prvy takt
    start = millis();
    for(int i=0; i<16; i++)
    {
      barLeft[i] = osudovaRightAndLeft2[i];
      barRight[i] =osudovaRightAndLeft2[i];
    }
    playBar();
    while(millis() - start <= tempo ){}
    //druhy takt
    start = millis();
    for(int i=0; i<16; i++)
    {
      barLeft[i] = osudovaRightAndLeft3[i];
      barRight[i] =osudovaRightAndLeft3[i];
    }
    playBar();
    while(millis() - start <= tempo ){}
    //treti takt
    start = millis();
    for(int i=0; i<16; i++)
    {
      barLeft[i] = osudovaRightAndLeft4[i];
      barRight[i] =osudovaRightAndLeft4[i];
    }
    playBar();
    while(millis() - start <= tempo ){}
    //stvrty takt
    sendData.end = 1;
    esp_now_send(camAddr, (uint8_t *) &sendData, sizeof(sendData));
  }
  if(myData.song == 3)
  {
    start = millis();
    tempo = 3000;
    sest = tempo / 16;
    for(int i=0; i<16; i++)
    {
      barLeft[i] = stupnicaRightAndLeft[i];
      barRight[i] = stupnicaRightAndLeft[i];
    }
    for(int i=0; i<2; i++)
    {
      positionLeft[i] = stupnicaRightAndLeftPosition[i];
      positionRight[i] = stupnicaRightAndLeftPosition[i];
    }
    playBar();
    sendData.end = 1;
    esp_now_send(camAddr, (uint8_t *) &sendData, sizeof(sendData));
  }
  if(myData.song == 4)
  {
    tempo = 2280;
    sest = tempo/16;
    if(myData.time == 1)
    {
      for(int i=0; i<16; i++)
      {
        barLeft[i] = havasiFreedomLeft1[i];
        barRight[i] = havasiFreedomRight1[i];
      }
      for(int i=0; i<2; i++)
      {
        positionLeft[i] = havasiFreedomLeftPosition1[i];
        positionRight[i] = havasiFreedomRightPosition1[i];
      }
    }
    if(myData.time == 5)
    {
      for(int i=0; i<16; i++)
      {
        barLeft[i] = havasiFreedomLeft5[i];
        barRight[i] = havasiFreedomRight5[i];
      }
    }
    //piaty takt
    if(myData.time == 6)
    {
      for(int i=0; i<16; i++)
      {
        barLeft[i] = havasiFreedomLeft5[i];
        barRight[i] = havasiFreedomRight6[i];
      }
    }
    // siesty takt
    if(myData.time == 7)
    {
      for(int i=0; i<16; i++)
      {
        barLeft[i] = havasiFreedomLeft5[i];
        barRight[i] = havasiFreedomRight7[i];
      }
    }
    //siedmy takt
    if(myData.time == 8)
    {
      for(int i=0; i<16; i++)
      {
        barLeft[i] = havasiFreedomLeft5[i];
        barRight[i] = havasiFreedomRight8[i];
      }
    }
    //osmy takt
    if(myData.time == 9)
    {
      for(int i=0; i<16; i++)
      {
        barLeft[i] = havasiFreedomLeft5[i];
        barRight[i] = havasiFreedomRight9[i];
      }
      for(int i=0; i<2; i++)
      {
        positionLeft[i] = havasiFreedomLeftPosition9[i];
      }
    }
    //deviaty takt
    if(myData.time ==10)
    {
      for(int i=0; i<16; i++)
      {
        barLeft[i] = havasiFreedomLeft5[i];
        barRight[i] = havasiFreedomRight10[i];
      }
    }
    //desiaty takt
    if(myData.time ==11)
    {
      for(int i=0; i<16; i++)
      {
        barLeft[i] = havasiFreedomLeft5[i];
        barRight[i] = havasiFreedomRight11[i];
      }
    }
    //jedenasty takt
    if(myData.time == 12)
    {
      for(int i=0; i<16; i++)
      {
        barLeft[i] = havasiFreedomLeft12[i];
        barRight[i] = havasiFreedomRight12[i];
      }
      for(int i=0; i<2; i++)
      {
        positionLeft[i] = havasiFreedomLeftPosition12[i];
        positionRight[i] = havasiFreedomRightPosition12[i];
      }
    }
    //dvanasty a posledny takt
    playBar();
  }
  if(myData.song == 5)
  {
    if(myData.time == 1)
    {
      tempo = 1950; 
      sest = tempo / 16; 
      for(int i=0; i<16; i++)
      {
        barLeft[i] = fireballLeft1[i];
        barRight[i] = fireballRight1[i];
      }
      for(int i=0; i<2; i++)
      {
        positionLeft[i] = fireballLeftPosition1[i];
        positionRight[i] = fireballRightPosition1[i];
      }
    }
    if(myData.time == 2)
    {
      for(int i=0; i<16; i++)
      {
        barRight[i] = fireballRight2[i];
      }
    }
    if(myData.time == 3)
    {
      for(int i=0; i<16; i++)
      {
        barRight[i] = fireballRight1[i];
      }
    }
    if(myData.time == 4)
    {
      
      for(int i=0; i<16; i++)
      {
        barRight[i] = fireballRight2[i];
      } 
    }
    if(myData.time == 5)
    {
      for(int i=0; i<16; i++)
      {
        barRight[i] = fireballRight5[i];
      }
      for(int i=0; i<2; i++)
      {
        positionRight[i] = fireballRightPosition5[i];
      }
    }
    if(myData.time == 6)
    {
      for(int i=0; i<16; i++)
      {
        barRight[i] = fireballRight6[i];
      }
    }
    if(myData.time == 7)
    {
      for(int i=0; i<16; i++)
      {
        barRight[i] = fireballRight5[i];
      }
    }
    if(myData.time == 8)
    {
      for(int i=0; i<16; i++)
      {
        barRight[i] = fireballRight8[i];
      }
    }
    if(myData.time == 9)
    {
      for(int i=0; i<16; i++)
      {
        barRight[i] = fireballRight5[i];
      }
    }
    if(myData.time == 10)
    {
      for(int i=0; i<16; i++)
      {
        barRight[i] = fireballRight6[i];
      }
    }
    if(myData.time == 11)
    {
      for(int i=0; i<16; i++) 
      {
        barRight[i] = fireballRight5[i];
      }
    }
    if(myData.time == 12)
    {
      rightHand.stepper->moveTo(stepsPerOctave * 2);
      while (rightHand.stepper->isRunning()) 
      {
      } 
      leftHand.stepper->moveTo(0);
      while (leftHand.stepper->isRunning()) 
      {
      }
    }
    playBar();
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
  /*
  start = millis();
  tempo = 890; 
  sest = tempo / 16; 
  for(int i=0; i<16; i++)
  {
    barLeft[i] = osudovaRightAndLeft1[i];
    barRight[i] = osudovaRightAndLeft1[i];
  }
  for(int i=0; i<2; i++) 
  {
    positionLeft[i] = osudovaRightAndLeftPosition[i];
    positionRight[i] = osudovaRightAndLeftPosition[i];
  }
  playBar();
  while(millis() - start <= tempo ){}
  //prvy takt
  start = millis();
  for(int i=0; i<16; i++)
  {
    barLeft[i] = osudovaRightAndLeft2[i];
    barRight[i] =osudovaRightAndLeft2[i];
  }
  playBar();
  while(millis() - start <= tempo ){}
  //druhy takt
  start = millis();
  for(int i=0; i<16; i++)
  {
    barLeft[i] = osudovaRightAndLeft3[i];
    barRight[i] =osudovaRightAndLeft3[i];
  }
  playBar();
  while(millis() - start <= tempo ){}
  //treti takt
  start = millis();
  for(int i=0; i<16; i++)
  {
    barLeft[i] = osudovaRightAndLeft4[i];
    barRight[i] =osudovaRightAndLeft4[i];
  }
  playBar();
  while(millis() - start <= tempo ){}
  //stvrty takt
  start = millis();
  tempo == 5830;
  sest = tempo / 16;
  for(int i=0; i<16; i++)
  {
    barLeft[i] = stupnicaRightAndLeft[i];
    barRight[i] = stupnicaRightAndLeft[i];
  }
  for(int i=0; i<2; i++)
  {
    positionLeft[i] = stupnicaRightAndLeftPosition[i];
    positionRight[i] = stupnicaRightAndLeftPosition[i];
  }
  playBar();
  while(millis() - start <= tempo){}

  delay(5000);
  */
}