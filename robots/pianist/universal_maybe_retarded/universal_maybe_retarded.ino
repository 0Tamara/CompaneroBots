#include <FastAccelStepper.h>
#include <Adafruit_PWMServoDriver.h>
#include <Wire.h>

#define SERVOMIN  125
#define SERVOMAX  575
#define numServos 8
#define stepsPerNote 984
#define stepsPerOctave 6840
#define speedInHz 15000
#define acceleration 40000
// casy
int tempo = 2000;
int sest = tempo / 16; 
int osm = tempo / 8;
int stv = tempo / 4;
int pol = tempo / 2;
int cel = tempo;

const int rezerva = 50; 
const int leftHandStepPin = 14; 
const int leftHandDirPin = 12; 
const int leftHandEnPin = 18;
const int rightHandStepPin = 15;
const int rightHandDirPin = 13;
const int rightHandEnPin = 19;

// kniznice
Adafruit_PWMServoDriver pca9685right(0x40, Wire);
Adafruit_PWMServoDriver pca9685left(0x41, Wire1);

FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepperLeft = NULL;
FastAccelStepper *stepperRight = NULL;

enum moveNotes { C=0, D=1, E=2, F=3, G=4, A=5, H=6 };
enum actualServos { NIC = -1, SERVO1 = 0, SERVO2 = 1, SERVO3 = 2, SERVO4 = 3, SERVO5 = 4, SERVO6 = 5, SERVO7 = 6, SERVO8 = 7 };

struct Hand
{
  int currentOctave;
  int currentNote;
  FastAccelStepper* stepper;
  unsigned long timeFromMoving;
  unsigned long lastTime;
  Adafruit_PWMServoDriver* pca9685; 
};

Hand leftHand = {
  .currentOctave = 0,
  .currentNote = 0,
  .stepper = NULL,
  .timeFromMoving = 0,
  .lastTime = 0,
  .pca9685 = &pca9685left,
  

};
Hand rightHand = {
  .currentOctave = 0,
  .currentNote = 0,
  .stepper = NULL,
  .timeFromMoving = 0,
  .lastTime = 0,
  .pca9685 = &pca9685right,
  
};
void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  Wire1.begin(32, 33);
  pca9685right.begin();
  pca9685left.begin();
  pca9685right.setPWMFreq(50);
  pca9685left.setPWMFreq(50); 
  for (int i = 0; i < numServos; i++){
    pca9685left.setPWM(i, 0, SERVOMIN);
    pca9685right.setPWM(i, 0, SERVOMIN); // 0 stupnov
  }

  engine.init();
  stepperLeft = engine.stepperConnectToPin(leftHandStepPin);
  stepperRight = engine.stepperConnectToPin(rightHandStepPin);
  leftHand.stepper = stepperLeft;
  rightHand.stepper = stepperRight;
  if (stepperRight == NULL || stepperLeft == NULL) {
    Serial.println("Chyba pri pripojeni krokovych motorov.");
    while (1);
  }

  stepperLeft->setDirectionPin(leftHandDirPin);
  stepperLeft->setEnablePin(leftHandEnPin);
  stepperLeft->setAutoEnable(true);

  stepperLeft->setSpeedInHz(speedInHz);
  stepperLeft->setAcceleration(acceleration);
  stepperLeft->setCurrentPosition(0);
  
  stepperRight->setDirectionPin(rightHandDirPin);
  stepperRight->setEnablePin(rightHandEnPin);
  stepperRight->setAutoEnable(true);

  stepperRight->setSpeedInHz(speedInHz);
  stepperRight->setAcceleration(acceleration);
  stepperRight->setCurrentPosition(stepsPerOctave * 3); 
}

int havasiFreedomRight[][6] = {
    //prvy takt
    {A, 2, cel, NIC, NIC, NIC},
    // druhy takt
    {A, 2, cel, NIC, NIC, NIC},
    //treti takt
    {A, 2, cel, NIC, NIC, NIC},
    //stvrty takt
    {A, 2, cel, NIC, NIC, NIC},
    //piaty takt
    {A, 2, osm, SERVO1, SERVO3, SERVO5},
    {A, 2, sest, NIC, NIC, NIC},
    {A, 2, osm, SERVO1, SERVO3, SERVO5},
    {A, 2, sest, NIC, NIC, NIC},
    {A, 2, stv, SERVO1, SERVO3, NIC},
    {A, 2, sest, SERVO2, NIC, NIC},
    {A, 2, sest, SERVO3, NIC, NIC},
    {A, 2, osm, SERVO4, NIC, NIC},
    {A, 2, sest, NIC, NIC, NIC},
    {A, 2, sest, SERVO5, NIC, NIC},
    //prvy takt
    {A, 2, stv, SERVO1, SERVO3, NIC},
    {A, 2, osm, NIC, NIC, NIC},
    {A, 2, stv, NIC, NIC, NIC},
    {A, 2, sest, SERVO2, NIC, NIC},
    {A, 2, sest, SERVO3, NIC, NIC},
    {A, 2, osm, SERVO2, SERVO4, NIC},
    {A, 2, osm, SERVO3, SERVO5, NIC},
    //druhy takt
    {A, 2, osm, SERVO1, SERVO3, SERVO5},
    {A, 2, sest, NIC, NIC, NIC},
    {A, 2, osm, SERVO1, SERVO3},
    {A, 2, sest, NIC, NIC, NIC},
    {A, 2, stv, NIC, NIC, NIC},
    {A, 2, sest, SERVO2, NIC, NIC},
    {A, 2, sest, SERVO3, NIC, NIC},
    {A, 2, osm, SERVO4, NIC, NIC},
    {A, 2, sest, NIC, NIC, NIC},
    {A, 2, sest, SERVO5, NIC, NIC},
    //treti takt
    {A, 2, stv, SERVO1, SERVO3, NIC},
    {A, 2, osm, NIC, NIC, NIC},
    {A, 2, stv, NIC, NIC, NIC},
    {A, 2, sest, SERVO1, NIC, NIC},
    {A, 2, sest, SERVO2, NIC, NIC},
    {A, 2, osm, SERVO1, SERVO3, NIC},
    {A, 2, osm, SERVO1, SERVO4, NIC},
    //stvrty takt
    {A, 2, osm, SERVO1, SERVO3, SERVO5},
    {A, 2, sest, NIC, NIC, NIC},
    {A, 2, osm, SERVO1, SERVO3, SERVO5},
    {A, 2, sest, NIC, NIC, NIC},
    {A, 2, stv, NIC, NIC, NIC},
    {A, 2, sest, SERVO2, NIC, NIC},
    {A, 2, sest, SERVO3, NIC, NIC},
    {A, 2, osm, SERVO2, SERVO4, NIC},
    {A, 2, sest, NIC, NIC, NIC},
    {A, 2, sest, SERVO3, SERVO5, NIC},
    //prvy takt
    {A, 2, stv, SERVO1, SERVO3, NIC},
    {A, 2, stv, NIC, NIC, NIC},
    {A, 2, sest, SERVO2, NIC, NIC},
    {A, 2, sest, SERVO3, NIC, NIC},
    {A, 2, osm, SERVO2, SERVO4, NIC},
    {A, 2, osm, SERVO3, SERVO5, NIC},
    //druhy takt
    {A, 2, osm, SERVO1, SERVO3, SERVO5},
    {A, 2, sest, NIC, NIC, NIC},
    {A, 2, osm, SERVO1, SERVO3, NIC},
    {A, 2, stv, NIC, NIC, NIC},
    {A, 2, sest, SERVO1, NIC, NIC},
    {A, 2, sest, SERVO2, NIC, NIC},
    {A, 2, osm, SERVO1, SERVO3, NIC},
    {A, 2, osm, SERVO2, SERVO4},
    // treti takt, tu je toten krizik sprosty
    {G, 2, osm, SERVO1, SERVO6, NIC },
    {G, 2, sest, NIC, NIC, NIC},
    {G, 2, osm, SERVO1, SERVO7, NIC },
    {G, 2, osm, SERVO1, SERVO7, NIC },
    {G, 2, sest, NIC, NIC, NIC},
    {G, 2, stv, SERVO1, SERVO6, NIC },
    {G, 2, stv, SERVO1, SERVO6, NIC },
    //aaa tu sa to uz opakovat bude, nemam nervy pisat toto, a asi to nebude 
};
int havasiFreedomLeft[][6] = {
    //prvy takt
    {A, 1, osm, SERVO1, NIC, NIC},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, osm, NIC, SERVO8, NIC},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, osm, NIC, SERVO8, NIC},
    {A, 1, stv, NIC, SERVO8, NIC},
    {A, 1, stv, NIC, SERVO8, NIC},
    //druhy takt
    {A, 1, osm, SERVO1, NIC, NIC},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, osm, NIC, SERVO8, NIC},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, osm, NIC, SERVO8, NIC},
    {A, 1, stv, NIC, SERVO8, NIC},
    {A, 1, stv, NIC, SERVO8, NIC},
    //treti takt
    {A, 1, osm, SERVO1, NIC, NIC},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, osm, NIC, SERVO8, NIC},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, osm, NIC, SERVO8, NIC},
    {A, 1, stv, NIC, SERVO8, NIC},
    {A, 1, stv, NIC, SERVO8, NIC},
    //stvrty takt
    {A, 1, osm, SERVO1, NIC, NIC},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, osm, NIC, SERVO8, NIC},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, osm, NIC, SERVO8, NIC},
    {A, 1, stv, NIC, SERVO8, NIC},
    {A, 1, stv, NIC, SERVO8, NIC},
    //piaty takt
    {A, 1, osm, SERVO1, SERVO8, NIC},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, osm, SERVO1, SERVO8, NIC},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, osm, SERVO1, SERVO8, NIC},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, osm, SERVO1, SERVO8, NIC},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, stv, SERVO1, SERVO8, NIC},
    //prvy takt, nechapem jak mozu byt tak hnusne tony dane, sak tu sa iba opakuje jedna oktava skoro dookola
    {A, 1, osm, SERVO1, SERVO8, NIC},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, osm, SERVO1, SERVO8, NIC},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, osm, SERVO1, SERVO8, NIC},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, osm, SERVO1, SERVO8, NIC},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, stv, SERVO1, SERVO8, NIC},
    //druhy takt
    {A, 1, osm, SERVO1, SERVO8, NIC},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, osm, SERVO1, SERVO8, NIC},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, osm, SERVO1, SERVO8, NIC},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, osm, SERVO1, SERVO8, NIC},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, stv, SERVO1, SERVO8, NIC},
    //druhy takt
    {A, 1, osm, SERVO1, SERVO8, NIC},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, osm, SERVO1, SERVO8, NIC},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, osm, SERVO1, SERVO8, NIC},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, osm, SERVO1, SERVO8, NIC},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, stv, SERVO1, SERVO8, NIC},
    //treti takt
    {A, 1, osm, SERVO1, SERVO8, NIC},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, osm, SERVO1, SERVO8, NIC},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, osm, SERVO1, SERVO8, NIC},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, osm, SERVO1, SERVO8, NIC},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, stv, SERVO1, SERVO8, NIC},
    //stvrty takt
    {F, 1, osm, SERVO1, SERVO8, NIC},
    {F, 1, sest, NIC, NIC, NIC},
    {F, 1, osm, SERVO1, SERVO8, NIC},
    {F, 1, sest, NIC, NIC, NIC},
    {F, 1, osm, SERVO1, SERVO8, NIC},
    {F, 1, sest, NIC, NIC, NIC},
    {F, 1, osm, SERVO1, SERVO8, NIC},
    {F, 1, sest, NIC, NIC, NIC},
    {F, 1, stv, SERVO1, SERVO8, NIC},
    //prvy takt
    {F, 1, osm, SERVO1, SERVO8, NIC},
    {F, 1, sest, NIC, NIC, NIC},
    {F, 1, osm, SERVO1, SERVO8, NIC},
    {F, 1, sest, NIC, NIC, NIC},
    {F, 1, osm, SERVO1, SERVO8, NIC},
    {F, 1, sest, NIC, NIC, NIC},
    {F, 1, osm, SERVO1, SERVO8, NIC},
    {F, 1, sest, NIC, NIC, NIC},
    {F, 1, stv, SERVO1, SERVO8, NIC},
    //druhy takt
    {F, 1, osm, SERVO1, SERVO8, NIC},
    {F, 1, sest, NIC, NIC, NIC},
    {F, 1, osm, SERVO1, SERVO8, NIC},
    {F, 1, sest, NIC, NIC, NIC},
    {F, 1, osm, SERVO1, SERVO8, NIC},
    {F, 1, sest, NIC, NIC, NIC},
    {F, 1, osm, SERVO1, SERVO8, NIC},
    {F, 1, sest, NIC, NIC, NIC},
    {F, 1, stv, SERVO1, SERVO8, NIC},
    //treti takt
    {E, 1, osm, SERVO1, SERVO8, NIC},
    {E, 1, sest, NIC, NIC, NIC},
    {E, 1, osm, SERVO1, SERVO8, NIC},
    {E, 1, sest, NIC, NIC, NIC},
    {E, 1, osm, SERVO1, SERVO8, NIC},
    {E, 1, sest, NIC, NIC, NIC},
    {E, 1, osm, SERVO1, SERVO8, NIC},
    {E, 1, sest, NIC, NIC, NIC},
    {E, 1, stv, SERVO1, SERVO8, NIC},
    //tu zasa by sa to opakovalo od zacatku, rozmyslam ze do toho dam for cyklus nech ten kod nema 500 riadkov
};  
void loop() {
  xTaskCreatePinnedToCore(
  [] (void *) {
    playMelody(leftHand, havasiFreedomLeft, sizeof(havasiFreedomLeft) / sizeof(havasiFreedomLeft[0]));
    vTaskDelete(NULL);
  }, "LeftHandTask", 4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(
  [] (void *) {
    playMelody(rightHand, havasiFreedomRight, sizeof(havasiFreedomRight) / sizeof(havasiFreedomRight[0]));
    vTaskDelete(NULL);
  }, "RightHandTask", 4096, NULL, 1, NULL, 1); //konec
  while (1);
}


int angleToPulse(int angle){
  return map(angle, 0, 180, SERVOMIN, SERVOMAX);
}

unsigned long moveToNote(Hand& hand, int targetNote, int targetOctave) {
  unsigned long start = millis();
  if (targetNote < C || targetNote > H || targetOctave < 0 || targetOctave > 8) {
    Serial.printf("Neplatny targetNote(%d) alebo targetOctave(%d).\n", targetNote, targetOctave);
    return 0;
  }
  int lastSteps = hand.currentOctave * stepsPerOctave + hand.currentNote * stepsPerNote;
  int steps = targetOctave * stepsPerOctave + targetNote * stepsPerNote;
  if (steps - lastSteps == 0) return 0;
  hand.stepper->moveTo(steps);
  while (hand.stepper->isRunning());
  hand.currentOctave = targetOctave;
  hand.currentNote = targetNote;
  return millis() - start;
}

void playNote(Hand& hand, int targetNote, int targetOctave, int wait, int note1, int note2, int note3) {
  hand.timeFromMoving = moveToNote(hand, targetNote, targetOctave);
  int notes[3] = {note1, note2, note3};
  hand.lastTime = millis();
  int noteCount = 0;
  for(int i = 0; i < 3; i++){
    if(notes[i] != -1 && notes[i] < numServos) {
        noteCount +=1;
    }
  }
  int holdTime = wait - (rezerva * noteCount + hand.timeFromMoving);
  if (holdTime < 0) {
    Serial.printf("Pozor, negativny holdTime(%d ms). Ides na 50 ms.\n", holdTime);
    holdTime = 50;
  }
  while (millis() - hand.lastTime <= holdTime) {
    for (int i = 0; i < 3; i++){
      if (notes[i] != -1 && notes[i] < numServos){
        hand.pca9685->setPWM(notes[i], 0, angleToPulse(90)); 
      }
    }
  }
  hand.lastTime = millis();
  while (millis() - hand.lastTime <= rezerva * noteCount) {
    for (int i = 0; i < 3; i++){
      if (notes[i] != -1 && notes[i] < numServos){
        hand.pca9685->setPWM(notes[i], 0, angleToPulse(0)); 
      }
    }
  }
}

void playMelody(Hand& hand, int melody[][6], int length) {
  for (int i = 0; i < length; i++) {
    int targetNote = melody[i][0];
    int targetOctave = melody[i][1];
    int wait = melody[i][2];
    int note1 = melody[i][3];
    int note2 = melody[i][4];
    int note3 = melody[i][5];
    playNote(hand, targetNote, targetOctave, wait, note1, note2, note3);
  }
}