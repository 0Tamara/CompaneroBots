#include <FastAccelStepper.h>
#include <Adafruit_PWMServoDriver.h>

#define numServos 8
#define stepsPerNote 123
#define stepsPerOctave 855
#define SERVOMIN 125
#define SERVOMAX 575
// casy
int time = 2000;
int osm = time / 8;
int stv = time / 4;
int pol = time / 2;
int cel = time;

int rezerva = 50; 
const int leftHandStepPin = 14; 
const int leftHandDirPin = 12; 
const int leftHandEnPin = 18;
const int rightHandStepPin = 15;
const int rightHandDirPin = 13;
const int rightHandEnPin = 19;

// kniznice
Adafruit_PWMServoDriver pca9685 = Adafruit_PWMServoDriver(0x40);
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
  int channelOffset = 0; 
};

Hand leftHand = {
  .currentOctave = 0,
  .currentNote = 0,
  .stepper = NULL,
  .pca9685 = &pca9685,
  .channelOffset = 0 
};
Hand rightHand = {
  .currentOctave = 0,
  .currentNote = 0,
  .stepper = NULL,
  .pca9685 = &pca9685,
  .channelOffset = 8 
};
void setup() {
  Serial.begin(115200);
  pca9685.begin();
  pca9685.setPWMFreq(50); 
  for (int i = 0; i < numServos * 2; i++){
    pca9685.setPWM(i, 0, SERVOMIN); // 0 stupnov
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

  stepperLeft->setSpeedInUs(700);
  stepperLeft->setAcceleration(4000);
  stepperLeft->setCurrentPosition(0);
  
  stepperRight->setDirectionPin(rightHandDirPin);
  stepperRight->setEnablePin(rightHandEnPin);
  stepperRight->setAutoEnable(true);

  stepperRight->setSpeedInUs(700);
  stepperRight->setAcceleration(4000);
  stepperRight->setCurrentPosition(0);
}

void loop() {
  xTaskCreatePinnedToCore(
  [] (void *) {
    playMelody(leftHand, melodyLeft1, sizeof(melodyLeft1) / sizeof(melodyLeft1[0]));
    vTaskDelete(NULL);
  }, "LeftHandTask", 4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(
  [] (void *) {
    playMelody(rightHand, melodyRight1, sizeof(melodyRight1) / sizeof(melodyRight1[0]));
    vTaskDelete(NULL);
  }, "RightHandTask", 4096, NULL, 1, NULL, 1); //konec
  while (1);
}


int angleToPulse(int angle){
  return map(angle, 0, 180, SERVOMIN, SERVOMAX);
}

unsigned long moveToNote(Hand& hand, int targetNote, int targetOctave) {
  unsigned long start = millis();
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
        hand.pca9685->setPWM(notes[i] + hand.channelOffset, 0, angleToPulse(90)); 
      }
    }
  }
  hand.lastTime = millis();
  while (millis() - hand.lastTime <= rezerva * noteCount) {
    for (int i = 0; i < 3; i++){
      if (notes[i] != -1 && notes[i] < numServos){
        hand.pca9685->setPWM(notes[i] + hand.channelOffset, 0, angleToPulse(0)); 
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
int melodyLeft1[][6] = {
    {D, 5, osm, NIC, NIC, SERVO2},
    {C, 4, stv, SERVO2, NIC, SERVO5}
    

};
int melodyRight1[][6] = {
    {E, 2, osm, NIC, NIC, SERVO1},
    {G, 1, stv, SERVO3, NIC, SERVO6}
};
