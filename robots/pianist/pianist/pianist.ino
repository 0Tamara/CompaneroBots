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

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.printf("myData = %d\n", myData);
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
  //prvy takt
  if(myData == 2)
  {
    start = millis();
    playBar();
    //druhy takt
  }
  playBar();
  if(myData == 3)
  {   
    start = millis();
    playBar();
    //treti takt
  }
  playBar();
  if(myData == 4)
  {
    start = millis();
    playBar();
    //stvrty takt
  }
  playBar();
  if(myData == 5)
  {
    for(int i=0; i<16; i++)
    {
      barLeft[i] = havasiFreedomLeft5[i];
      barRight[i] = havasiFreedomRight5[i];
    }
  }
  playBar();
  //piaty takt
  if(myData == 6)
  {
    for(int i=0; i<16; i++)
    {
      barLeft[i] = havasiFreedomLeft5[i];
      barRight[i] = havasiFreedomRight6[i];
    }
  }
  playBar();
  // siesty takt
  if(myData == 7)
  {
    for(int i=0; i<16; i++)
    {
      barLeft[i] = havasiFreedomLeft5[i];
      barRight[i] = havasiFreedomRight7[i];
    }
  }
  playBar();
  //siedmy takt
  if(myData == 8)
  {
    for(int i=0; i<16; i++)
    {
      barLeft[i] = havasiFreedomLeft5[i];
      barRight[i] = havasiFreedomRight8[i];
    }
  }
  playBar();
  //osmy takt
  if(myData == 9)
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
  playBar();
  //deviaty takt
  if(myData == 10)
  {
    for(int i=0; i<16; i++)
    {
      barLeft[i] = havasiFreedomLeft5[i];
      barRight[i] = havasiFreedomRight10[i];
    }
  }
  playBar();
  //desiaty takt
  if(myData == 11)
  {
    for(int i=0; i<16; i++)
    {
      barLeft[i] = havasiFreedomLeft1[i];
      barRight[i] = havasiFreedomRight1[i];
    }
  }
  playBar();
  //jedenasty takt
  if(myData == 12)
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
  playBar();
  //dvanasty a posledny takt
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

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
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
  rightHand.stepper->moveTo(0);
  while (rightHand.stepper->isRunning()) {
  } 
  leftHand.stepper->moveTo(0);
  while (leftHand.stepper->isRunning()) {
  }
}

void loop() {
}