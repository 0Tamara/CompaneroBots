#include <esp_log.h>
#include <FastAccelStepper.h>
#include <Adafruit_PWMServoDriver.h>
#include <Wire.h>
#include <esp_now.h>
#include <WiFi.h>

#define SERVOMIN  125
#define SERVOMAX  575
#define numServos 16
#define stepsPerNote 984
#define stepsPerOctave 6840
#define speedInHz 15000
#define acceleration 40000

const int leftHandStepPin = 5; 
const int leftHandDirPin = 16; 
const int leftHandEnPin = 18;
const int rightHandStepPin = 4;
const int rightHandDirPin = 17;
const int rightHandEnPin = 15;
// casy
byte myData;
int targetNoteRight = 0;
int targetOctaveRight = 0;
int targetNoteLeft = 0;
int targetOctaveLeft = 0;
int stepsRight = 0;
int stepsLeft = 0;
unsigned long timeBeforeMoving = 0;
int timeFromMoving = 0;
int takt = 2280;
int tempo = 2100;
int sest = tempo / 16; 
unsigned long start = millis();
const int offset = 50; //konstanta, o tolko sa bude musiet pohnut kym sa dostane na klaviaturu
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
  rightHand.stepper->moveTo(0);
  while (rightHand.stepper->isRunning()) {
  } 
  leftHand.stepper->moveTo(0);
  while (leftHand.stepper->isRunning()) {
  }
}
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
  0b10101010, // osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b10101010, // osm
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
  0b10101010, // osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b10101010, // osm
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
  0b01000010, //osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b01000010, //osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b01000010, //osm
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
  0b00000001, //sest
  0b00000001, //osm
  0b00000000, 
  0b00000001, //sest
  0b00000000, //stv pomlcka
  0b00000000, 
  0b00000000, 
  0b00000000, 
};
int fireballLeftPosition1[] = {F, 1};
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
int barLeft[16];
int barRight[16];
int positionLeft[2];
int positionRight[2];

void playBar(){
  Serial.println("Start function");
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
    Serial.println("for");
    int wait = sest;
    byte notesRight = barRight[i];
    byte notesLeft = barLeft[i];
    
    if(!(rightHand.stepper->isRunning() || leftHand.stepper->isRunning()))
    {
      for (int j = 0; j < 8; j++)
      {
        if (notesRight & 1<<j)
        {
          pca9685right.setPWM(j+8, 0, rightHand.pressValue);
        }
        if (notesLeft & 1<<j)
        {
          pca9685left.setPWM(j+8, 0, leftHand.pressValue);
        }
      }
      Serial.println("stlacene");
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
}

void loop() {
  start = millis();
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
  playBar();
  while(millis()-start < 2280);
  start = millis();
  for(int i=0; i<16; i++)
  {
    barLeft[i] = havasiFreedomLeft5[i];
    barRight[i] = havasiFreedomRight5[i];
  }
  for(int i=0; i<2; i++)
  {
    positionLeft[i] = havasiFreedomLeftPosition9[i];
  }
  playBar();
  while(millis()-start < 2280);

  start = millis();
  for(int i=0; i<16; i++)
  {
    barRight[i] = havasiFreedomRight6[i];
  }
  playBar();
  while(millis()-start < 2280);
}
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));

  start = millis();
  
  if(myData == 1)
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
  playBar();
}