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
// casy
byte myData;
int targetNoteRight = 0
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
const int leftHandStepPin = 5; 
const int leftHandDirPin = 16; 
const int leftHandEnPin = 18;
const int rightHandStepPin = 4;
const int rightHandDirPin = 17;
const int rightHandEnPin = 15;

// kniznice
Adafruit_PWMServoDriver pca9685right(0x41, Wire);
Adafruit_PWMServoDriver pca9685left(0x40, Wire);

FastAccelStepperEngine engine = FastAccelStepperEngine();

enum moveNotes { C=0, D=1, E=2, F=3, G=4, A=5, H=6 };
enum actualServos { NIC = 0, SERVO1 = 15, SERVO2 = 14, SERVO3 = 13, SERVO4 = 12, SERVO5 = 11, SERVO6 = 10, SERVO7 = 9, SERVO8 = 8 };

struct Hand
{
  int currentOctave;
  int currentNote;
  FastAccelStepper* stepper;
  unsigned long timeFromMoving;
  unsigned long lastTime;
  //Adafruit_PWMServoDriver* pca9685; 
  int pressValue;
  int releaseValue;
};

Hand leftHand = {
  .currentOctave = 0,
  .currentNote = 0,
  .stepper = NULL,
  .timeFromMoving = 0,
  .lastTime = 0,
  //.pca9685 = &pca9685left,
  .pressValue = SERVOMAX - 100,
  .releaseValue = SERVOMAX,
};
Hand rightHand = {
  .currentOctave = 0,
  .currentNote = 0,
  .stepper = NULL,
  .timeFromMoving = 0,
  .lastTime = 0,
  //.pca9685 = &pca9685right,
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
int havasiFreedomRight1[][1]{
  {0b00000000}, //cel pomlcka
  {0b00000000},
  {0b00000000},
  {0b00000000},
  {0b00000000},
  {0b00000000},
  {0b00000000},
  {0b00000000},
  {0b00000000},
  {0b00000000},
  {0b00000000},
  {0b00000000},
  {0b00000000},
  {0b00000000},
  {0b00000000},
  {0b00000000},
};

int havasiFreedomLeft1[][1] = {
  {0b10000000}, //osm
  {0b00000000},
  {0b00000000}, // sest pomlcka
  {0b00000001}, //osm
  {0b00000000}, 
  {0b00000000}, //sest pomlcka
  {0b10000001}, //osm
  {0b00000000},
  {0b10000001}, //stv
  {0b00000000},
  {0b00000000},
  {0b00000000}, 
  {0b10000001}, //stv
  {0b00000000},
  {0b00000000},
  {0b00000000},
};
int havasiFreedomRight5[][1] = {
  {0b10101010}, // osm
  {0b00000000},
  {0b00000000}, //sest pomlcka
  {0b10101010}, // osm
  {0b00000000},
  {0b00000000}, //sest pomlcka
  {0b10100000}, // stv
  {0b00000000},
  {0b00000000},
  {0b00000000},
  {0b10000000}, //sest
  {0b01000000}, //sest
  {0b00100000}, //sest
  {0b01010000}, //osm
  {0b00000000}, 
  {0b00001000}, //sest
}; 
int havasiFreedomLeft5[][1]{
  {0b10000001}, //osm
  {0b00000000},
  {0b00000000}, //sest pomlcka
  {0b10000001}, //osm
  {0b00000000},
  {0b00000000}, //sest pomlcka
  {0b10000001}, //osm
  {0b00000000},
  {0b00000000}, //sest pomlcka
  {0b10000001}, //osm
  {0b00000000},
  {0b00000000}, //sest pomlcka
  {0b10000001}, //stv
  {0b00000000},
  {0b00000000}, 
  {0b00000000},
};
int havasiFreedomRight6[][1]{
  {0b10100000}, //stv
  {0b00000000},
  {0b00000000},
  {0b00000000},
  {0b00000000}, // osm pomlcka
  {0b00000000},
  {0b00000000}, //stvrtova pomlcka
  {0b00000000},
  {0b00000000},
  {0b00000000},
  {0b01000000}, //sest
  {0b00100000}, //sest
  {0b01010000}, //osm
  {0b00000000},
  {0b00101000}, //sest
  {0b00000000},
};
int havasiFreedomRight7[][1]{
  //este len zacatok a uz si uvedomujem jak krasne vonka svieti slnko
  {0b10101000}, // osm
  {0b00000000},
  {0b00000000}, //sest pomlcka
  {0b10100000}, // osm
  {0b00000000},
  {0b00000000}, //sest pomlcka
  {0b00000000}, // stv pomlcka
  {0b00000000},
  {0b00000000},
  {0b00000000},
  {0b10000000}, //sest
  {0b01000000}, //sest
  {0b10100000}, //osm
  {0b00000000},
  {0b00000000}, // sest pomlcka
  {0b01010000}, // sest
  
};
int havasiFreedomRight8[][1]{
  {0b10100000}, //stv
  {0b00000000},
  {0b00000000},
  {0b00000000},
  {0b00000000}, //osm pomlcka
  {0b00000000},
  {0b00000000}, //stv pomlcka
  {0b00000000},
  {0b00000000},
  {0b00000000},
  {0b10000000}, //sest
  {0b01000000}, //sest
  {0b10100000}, //osm
  {0b00000000},
  {0b10010000}, //osm
  {0b00000000},
};

int havasiFreedomLeftPosition9[]{F, 1};
int havasiFreedomRight9[][1]{
  //uz len kuuus... 
  {0b10101010}, // osm
  {0b00000000},
  {0b00000000}, //sest pomlcka
  {0b10101010}, // osm
  {0b00000000},
  {0b00000000}, //sest pomlcka
  {0b00000000}, // stv
  {0b00000000},
  {0b00000000},
  {0b00000000},
  {0b10000000}, //sest
  {0b01000000}, //sest
  {0b10100000}, //osm
  {0b00000000},
  {0b00000000}, //sest pomlcka 
  {0b01010000}, //sest
};
int havasiFreedomRight10[][1]{
  {0b10100000}, //stv
  {0b00000000},
  {0b00000000},
  {0b00000000},
  {0b00000000}, //osm pomlcka
  {0b00000000},
  {0b00000000}, //stv pomlcka
  {0b00000000},
  {0b00000000},
  {0b00000000},
  {0b10000000}, //sest
  {0b01000000}, //sest
  {0b10100000}, //osm
  {0b00000000},
  {0b01010000}, //osm
  {0b00000000},
};
int havasiFreedomRight11[][1]{
  {0b10101000}, // osm
  {0b00000000},
  {0b00000000}, //sest pomlcka
  {0b10100000}, // osm
  {0b00000000},
  {0b00000000}, //sest pomlcka
  {0b00000000}, // stv pomlcka
  {0b00000000},
  {0b00000000},
  {0b00000000},
  {0b10000000}, //sest
  {0b01000000}, //sest
  {0b10100000}, //osm
  {0b00000000},
  {0b01010000}, // osm
  {0b00000000}, 
};
int havasiFreedomRightPosition12[]{G, 1};
int havasiFreedomLeftPosition12[]{E, 1};
int havasiFreedomRight12[][1]{
  //jak to bolo? Tech tisic mil tech tisic mil ma jeden daco, a jeden cil
  {0b01000010}, //osm
  {0b00000000},
  {0b00000000}, //sest pomlcka
  {0b01000010}, //osm
  {0b00000000},
  {0b00000000}, //sest pomlcka
  {0b01000010}, //osm
  {0b00000000},
  {0b01000010}, //stv
  {0b00000000}, 
  {0b00000000}, 
  {0b00000000}, 
  {0b01000010}, //stv
  {0b00000000}, 
  {0b00000000}, 
  {0b00000000}, 
};
int havasiFreedomLeft12[][1]{
  {0b10000001}, //osm
  {0b00000000},
  {0b00000000}, //sest pomlcka
  {0b10000001}, //osm
  {0b00000000},
  {0b00000000}, //sest pomlcka
  {0b10000001}, //osm
  {0b00000000},
  {0b10000001}, //stv
  {0b00000000}, 
  {0b00000000}, 
  {0b00000000}, 
  {0b10000001}, //stv
  {0b00000000}, 
  {0b00000000}, 
  {0b00000000},   
};
int fireballRightPosition1[]{G, 1};
int fireballRight1[][1]{
  {0b10000000}, //stv
  {0b00000000}, 
  {0b00000000}, 
  {0b00000000},  
  {0b00000001}, //osm
  {0b00000000}, 
  {0b00000001}, //osm
  {0b00000000}, 
  {0b00000001}, //sest
  {0b00000001}, //osm
  {0b00000000}, 
  {0b00000001}, //sest
  {0b00000000}, //stv pomlcka
  {0b00000000}, 
  {0b00000000}, 
  {0b00000000}, 
};
int fireballLeftPosition1[]{F, 1};
int fireballLeft1[][1]{
  {0b10000000}, //stv
  {0b00000000}, 
  {0b00000000}, 
  {0b00000000},  
  {0b00001000}, //osm
  {0b00000000},  
  {0b00000010}, //osm
  {0b00000000},  
  {0b00000001}, //stv
  {0b00000000}, 
  {0b00000000}, 
  {0b00000000},  
  {0b00000010}, //osm
  {0b00000000},  
  {0b00001000}, //osm
  {0b00000000},  
};
void havasiFreedom1(){
  targetNoteRight = havasiFreedomRightPosition1[0];
  targetOctaveRight = havasiFreedomRightPosition1[1];
  targetNoteLeft = havasiFreedomLeftPosition1[0];
  targetOctaveLeft = havasiFreedomLeftPosition1[1];
  stepsRight = stepsPerNote * targetNoteRight + stepsPerNote * (targetOctaveRight - 1);
  stepsLeft = stepsPerNote * targetNoteLeft + stepsPerNote * (targetOctaveLeft - 1);
  rightHand.stepper->moveTo(stepsRight);
  leftHand.stepper->moveTo(stepsLeft);
  timeBeforeMoving = millis();  
  while (rightHand.stepper->isRunning() || leftHand.stepper->isRunning()){};
  timeFromMoving = millis() - timeFromMoving;
  for (int i = 0; i < 16; i++)
  {
    int wait = sest;
    if(timeFromMoving <= wait)
    {
      timeFromMoving -= wait;
      break;
    }
    else
    {  
    start -= timeFromMoving;
    timeFromMoving = 0;
    }
    byte notesRight = havasiFreedomRight1[i][0];
    byte notesLeft = havasiFreedomLeft1[i][0];
    
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

void havasiFreedom5(){
  targetNoteRight = havasiFreedomRightPosition1[0];
  targetOctaveRight = havasiFreedomRightPosition1[1];
  targetNoteLeft = havasiFreedomLeftPosition1[0];
  targetOctaveLeft = havasiFreedomLeftPosition1[1];
  stepsRight = stepsPerNote * targetNoteRight + stepsPerNote * (targetOctaveRight - 1);
  stepsLeft = stepsPerNote * targetNoteLeft + stepsPerNote * (targetOctaveLeft - 1);
  rightHand.stepper->moveTo(stepsRight);
  leftHand.stepper->moveTo(stepsLeft);
  timeBeforeMoving = millis();  
  while (rightHand.stepper->isRunning() || leftHand.stepper->isRunning()){};
  timeFromMoving = millis() - timeFromMoving;

  for (int i = 0; i < 16; i++)
  {
      
    int wait = sest;
    byte notesRight = havasiFreedomRight5[i][0];
    byte notesLeft = havasiFreedomLeft5[i][0];
    if(timeFromMoving <= wait)
    {
      timeFromMoving -= wait;
      break;
    }
    else
    {  
    start -= timeFromMoving;
    timeFromMoving = 0;
    }
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
    
    delay(75);
    for (int j = 0; j < 8; j++)
    {
      pca9685right.setPWM(j+8, 0, rightHand.releaseValue);
      pca9685left.setPWM(j+8, 0, leftHand.releaseValue);
    }
    while (millis() - start <= wait)
    {
    }
  }
}
void havasiFreedom6(){
  targetNoteRight = havasiFreedomRightPosition1[0];
  targetOctaveRight = havasiFreedomRightPosition1[1];
  targetNoteLeft = havasiFreedomLeftPosition1[0];
  targetOctaveLeft = havasiFreedomLeftPosition1[1];
  stepsRight = stepsPerNote * targetNoteRight + stepsPerNote * (targetOctaveRight - 1);
  stepsLeft = stepsPerNote * targetNoteLeft + stepsPerNote * (targetOctaveLeft - 1);
  rightHand.stepper->moveTo(stepsRight);
  leftHand.stepper->moveTo(stepsLeft);
  timeBeforeMoving = millis();  
  while (rightHand.stepper->isRunning() || leftHand.stepper->isRunning()){};
  timeFromMoving = millis() - timeFromMoving;
  for (int i = 0; i < 16; i++)
  {
    if(timeFromMoving <= wait)
    {
      timeFromMoving -= wait;
      break;
    }
    else
    {  
    start -= timeFromMoving;
    timeFromMoving = 0;
    }
    int wait = sest;
    byte notesRight = havasiFreedomRight6[i][0];
    byte notesLeft = havasiFreedomLeft5[i][0];
    
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
    
    delay(75);
    for (int j = 0; j < 8; j++)
    {
      pca9685right.setPWM(j+8, 0, rightHand.releaseValue);
      pca9685left.setPWM(j+8, 0, leftHand.releaseValue);
    }
    while (millis() - start <= wait)
    {
    }
  }
}
void havasiFreedom7(){
  targetNoteRight = havasiFreedomRightPosition1[0];
  targetOctaveRight = havasiFreedomRightPosition1[1];
  targetNoteLeft = havasiFreedomLeftPosition1[0];
  targetOctaveLeft = havasiFreedomLeftPosition1[1];
  stepsRight = stepsPerNote * targetNoteRight + stepsPerNote * (targetOctaveRight - 1);
  stepsLeft = stepsPerNote * targetNoteLeft + stepsPerNote * (targetOctaveLeft - 1);
  rightHand.stepper->moveTo(stepsRight);
  leftHand.stepper->moveTo(stepsLeft);
  timeBeforeMoving = millis();  
  while (rightHand.stepper->isRunning() || leftHand.stepper->isRunning()){};
  timeFromMoving = millis() - timeFromMoving;
  for (int i = 0; i < 16; i++)
  {
    if(timeFromMoving <= wait)
    {
      timeFromMoving -= wait;
      break;
    }
    else
    {  
    start -= timeFromMoving;
    timeFromMoving = 0;
    }
    int wait = sest;
    byte notesRight = havasiFreedomRight7[i][0];
    byte notesLeft = havasiFreedomLeft5[i][0];
    
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
    
    delay(75);
    for (int j = 0; j < 8; j++)
    {
      pca9685right.setPWM(j+8, 0, rightHand.releaseValue);
      pca9685left.setPWM(j+8, 0, leftHand.releaseValue);
    }
    while (millis() - start <= wait)
    {
    }
  }
}
void havasiFreedom8(){
  targetNoteRight = havasiFreedomRightPosition1[0];
  targetOctaveRight = havasiFreedomRightPosition1[1];
  targetNoteLeft = havasiFreedomLeftPosition1[0];
  targetOctaveLeft = havasiFreedomLeftPosition1[1];
  stepsRight = stepsPerNote * targetNoteRight + stepsPerNote * (targetOctaveRight - 1);
  stepsLet = stepsPerNote * targetNoteLeft + stepsPerNote * (targetOctaveLeft - 1);
  rightHand.stepper->moveTo(stepsRight);
  leftHand.stepper->moveTo(stepsLeft);
  timeBeforeMoving = millis();  
  while (rightHand.stepper->isRunning() || leftHand.stepper->isRunning()){};
  timeFromMoving = millis() - timeFromMoving;
  for (int i = 0; i < 16; i++)
  {
    if(timeFromMoving <= wait)
    {
      timeFromMoving -= wait;
      break;
    }
    else
    {  
    start -= timeFromMoving;
    timeFromMoving = 0;
    }
    int wait = sest;
    byte notesRight = havasiFreedomRight8[i][0];
    byte notesLeft = havasiFreedomLeft5[i][0];
    
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
    
    delay(75);
    for (int j = 0; j < 8; j++)
    {
      pca9685right.setPWM(j+8, 0, rightHand.releaseValue);
      pca9685left.setPWM(j+8, 0, leftHand.releaseValue);
    }
    while (millis() - start <= wait)
    {
    }
  }
}
void havasiFreedom9(){
  targetNoteRight = havasiFreedomRightPosition1[0];
  targetOctaveRight = havasiFreedomRightPosition1[1];
  targetNoteLeft = havasiFreedomLeftPosition9[0];
  targetOctaveLeft = havasiFreedomLeftPosition9[1];
  stepsRight = stepsPerNote * targetNoteRight + stepsPerNote * (targetOctaveRight - 1);
  stepsLeft = stepsPerNote * targetNoteLeft + stepsPerNote * (targetOctaveLeft - 1);
  rightHand.stepper->moveTo(stepsRight);
  leftHand.stepper->moveTo(stepsLeft);
  timeBeforeMoving = millis();  
  while (rightHand.stepper->isRunning() || leftHand.stepper->isRunning()){};
  timeFromMoving = millis() - timeFromMoving;
  for (int i = 0; i < 16; i++)
  {
    if(timeFromMoving <= wait)
    {
      timeFromMoving -= wait;
      break;
    }
    else
    {  
    start -= timeFromMoving;
    timeFromMoving = 0;
    }
    int wait = sest;
    byte notesRight = havasiFreedomRight9[i][0];
    byte notesLeft = havasiFreedomLeft5[i][0];
    
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
    delay(75);
    for (int j = 0; j < 8; j++)
    {
      pca9685right.setPWM(j+8, 0, rightHand.releaseValue);
      pca9685left.setPWM(j+8, 0, leftHand.releaseValue);
    }
    while (millis() - start <= wait)
    {
    }
  }
}
void havasiFreedom10(){
  targetNoteRight = havasiFreedomRightPosition1[0];
  targetOctaveRight = havasiFreedomRightPosition1[1];
  targetNoteLeft = havasiFreedomLeftPosition9[0];
  targetOctaveLeft = havasiFreedomLeftPosition9[1];
  stepsRight = stepsPerNote * targetNoteRight + stepsPerNote * (targetOctaveRight - 1);
  stepsLeft = stepsPerNote * targetNoteLeft + stepsPerNote * (targetOctaveLeft - 1);
  rightHand.stepper->moveTo(stepsRight);
  leftHand.stepper->moveTo(stepsLeft);
  timeBeforeMoving = millis();  
  while (rightHand.stepper->isRunning() || leftHand.stepper->isRunning()){};
  timeFromMoving = millis() - timeFromMoving;
  for (int i = 0; i < 16; i++)
  {
    if(timeFromMoving <= wait)
    {
      timeFromMoving -= wait;
      break
    }
    else
    {  
    start -= timeFromMoving;
    timeFromMoving = 0;
    }
    int wait = sest;
    byte notesRight = havasiFreedomRight10[i][0];
    byte notesLeft = havasiFreedomLeft5[i][0];
    
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
    
    delay(75);
    for (int j = 0; j < 8; j++)
    {
      pca9685right.setPWM(j+8, 0, rightHand.releaseValue);
      pca9685left.setPWM(j+8, 0, leftHand.releaseValue);
    }
    while (millis() - start <= wait)
    {
    }
  }
}
void havasiFreedom11(){
  targetNoteRight = havasiFreedomRightPosition1[0];
  targetOctaveRight = havasiFreedomRightPosition1[1];
  targetNoteLeft = havasiFreedomLeftPosition9[0];
  targetOctaveLeft = havasiFreedomLeftPosition9[1];
  stepsRight = stepsPerNote * targetNoteRight + stepsPerNote * (targetOctaveRight - 1);
  stepsLeft = stepsPerNote * targetNoteLeft + stepsPerNote * (targetOctaveLeft - 1);
  rightHand.stepper->moveTo(stepsRight);
  leftHand.stepper->moveTo(stepsLeft);
  timeBeforeMoving = millis();  
  while (rightHand.stepper->isRunning() || leftHand.stepper->isRunning()){};
  timeFromMoving = millis() - timeFromMoving;
  for (int i = 0; i < 16; i++)
  {
    unsigned long start = millis();
    int wait = sest;
    byte notesRight = havasiFreedomRight11[i][0];
    byte notesLeft = havasiFreedomLeft5[i][0];
    
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
    
    delay(75);
    for (int j = 0; j < 8; j++)
    {
      pca9685right.setPWM(j+8, 0, rightHand.releaseValue);
      pca9685left.setPWM(j+8, 0, leftHand.releaseValue);
    }
    while (millis() - start <= wait)
    {
    }
  }
}
void havasiFreedom12(){
  targetNoteRight = havasiFreedomRightPosition12[0];
  targetOctaveRight = havasiFreedomRightPosition12[1];
  targetNoteLeft = havasiFreedomLeftPosition12[0];
  targetOctaveLeft = havasiFreedomLeftPosition12[1];
  stepsRight = stepsPerNote * targetNoteRight + stepsPerNote * (targetOctaveRight - 1);
  stepsLeft = stepsPerNote * targetNoteLeft + stepsPerNote * (targetOctaveLeft - 1);
  rightHand.stepper->moveTo(stepsRight);
  leftHand.stepper->moveTo(stepsLeft);
  timeBeforeMoving = millis();  
  while (rightHand.stepper->isRunning() || leftHand.stepper->isRunning()){};
  timeFromMoving = millis() - timeFromMoving;
  for (int i = 0; i < 16; i++)
  {
    if(timeFromMoving <= wait)
    {
      timeFromMoving -= wait;
      break
    }
    else
    {  
    start -= timeFromMoving;
    timeFromMoving = 0;
    }
    int wait = sest;
    byte notesRight = havasiFreedomRight12[i][0];
    byte notesLeft = havasiFreedomLeft12[i][0];
    
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
    
    delay(75);
    for (int j = 0; j < 8; j++)
    {
      pca9685right.setPWM(j+8, 0, rightHand.releaseValue);
      pca9685left.setPWM(j+8, 0, leftHand.releaseValue);
    }
    while (millis() - start <= wait)
    {
    }
  }
}
void fireball1(){
  targetNoteRight = fireballRightPosition1[0];
  targetOctaveRight = fireballRightPosition1[1];
  targetNoteLeft = fireballLeftPosition1[0];
  targetOctaveLeft = fireballLeftPosition1[1];
  stepsRight = stepsPerNote * targetNoteRight + stepsPerNote * (targetOctaveRight - 1);
  stepsLeft = stepsPerNote * targetNoteLeft + stepsPerNote * (targetOctaveLeft - 1);
  rightHand.stepper->moveTo(stepsRight);
  leftHand.stepper->moveTo(stepsLeft);
  timeBeforeMoving = millis();  
  while (rightHand.stepper->isRunning() || leftHand.stepper->isRunning()){};
  timeFromMoving = millis() - timeFromMoving;
  for (int i = 0; i < 16; i++)
  {
    if(timeFromMoving <= wait)
    {
      timeFromMoving -= wait;
      break;
    }
    else
    {  
    start -= timeFromMoving;
    timeFromMoving = 0;
    }
    int wait = sest;
    byte notesRight = fireballRight1[i][0];
    byte notesLeft = fireballLeft1[i][0];
    
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
    
    delay(75);
    for (int j = 0; j < 8; j++)
    {
      pca9685right.setPWM(j+8, 0, rightHand.releaseValue);
      pca9685left.setPWM(j+8, 0, leftHand.releaseValue);
    }
    while (millis() - start <= wait)
    {
    }
  }
}
void loop() {
  if(myData == 1)
  {
    start = millis();
    havasiFreedom1();
  }
  if(myData == 2)
  {
    start = millis();
    havasiFreedom1();
  }
  if(myData == 3)
  {
    start = millis();
    havasiFreedom1();
  }
  if(myData == 4)
  {
    start = millis();
    havasiFreedom1();
  }
  if(myData == 5)
  {
    start = millis();
    havasiFreedom5();
  }
  if(myData == 6)
  {
    start = millis();
    havasiFreedom6();
  }
  if(myData == 7)
  {
    start = millis();
    havasiFreedom7();
  }
  if(myData == 8)
  {
    start = millis();
    havasiFreedom8();
  }
  if(myData == 9)
  {
    start = millis();
    havasiFreedom9();
  }
  if(myData == 10)
  {
    start = millis();
    havasiFreedom10();
  }
  if(myData == 11)
  {
    start = millis();
    havasiFreedom11();
  }
  if(myData == 12)
  {
    start = millis();
    havasiFreedom12();
  }
  //  vTaskDelete(NULL);
  //}, "LeftHandTask", 4096, NULL, 1, NULL, 0);
  /*
  delay(500);
  fireball1(); 
  */
}
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  if(myData == 1)
  {
    start = millis();
    havasiFreedom1();
  }
}