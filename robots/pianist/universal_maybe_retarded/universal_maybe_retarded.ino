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
int tempo = 2208;
int sest = tempo / 16; 
int osm = tempo / 8;
int stv = tempo / 4;
int pol = tempo / 2;
int cel = tempo;

const int offset = 50; //konstanta, o tolko sa bude musiet pohnut kym sa dostane na klaviaturu
const int rezerva = 20; 
const int leftHandStepPin = 5; 
const int leftHandDirPin = 16; 
const int leftHandEnPin = 18;
const int rightHandStepPin = 4;
const int rightHandDirPin = 17;
const int rightHandEnPin = 15;

// kniznice
Adafruit_PWMServoDriver pca9685right(0x40, Wire);
Adafruit_PWMServoDriver pca9685left(0x41, Wire);

FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepperLeft = NULL;
FastAccelStepper *stepperRight = NULL;

enum moveNotes { C=0, D=1, E=2, F=3, G=4, A=5, H=6 };
enum actualServos { NIC = -1, SERVO1 = 15, SERVO2 = 14, SERVO3 = 13, SERVO4 = 12, SERVO5 = 11, SERVO6 = 10, SERVO7 = 9, SERVO8 = 8 };

struct Hand
{
  int currentOctave;
  int currentNote;
  FastAccelStepper* stepper;
  unsigned long timeFromMoving;
  unsigned long lastTime;
  Adafruit_PWMServoDriver* pca9685; 
  int pressValue;
  int releaseValue;
};

Hand leftHand = {
  .currentOctave = 0,
  .currentNote = 0,
  .stepper = NULL,
  .timeFromMoving = 0,
  .lastTime = 0,
  .pca9685 = &pca9685left,
  .pressValue = SERVOMIN + 100,
  .releaseValue = SERVOMIN,
};
Hand rightHand = {
  .currentOctave = 0,
  .currentNote = 0,
  .stepper = NULL,
  .timeFromMoving = 0,
  .lastTime = 0,
  .pca9685 = &pca9685right,
  .pressValue = SERVOMAX - 100,
  .releaseValue = SERVOMAX,
  
};
void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  pca9685right.begin();
  pca9685left.begin();
  pca9685right.setPWMFreq(50);
  pca9685left.setPWMFreq(50); 
  for (int i = 7; i <= numServos; i++){
    rightHand.pca9685->setPWM(i, 0, SERVOMAX);
    leftHand.pca9685->setPWM(i, 0, SERVOMIN); // 0 stupnov
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
  stepperRight->setCurrentPosition((stepsPerOctave * 2)); 
  stepperRight->moveTo(0);
  while (stepperRight->isRunning()) {
  } 
  stepperLeft->moveTo(0);
  while (stepperLeft->isRunning()) {
  }
}

unsigned long moveToNote(Hand& hand, int targetNote, int targetOctave) {
  unsigned long start = millis();
  if (targetNote < C || targetNote > H || targetOctave < 0 || targetOctave > 8) {
    Serial.printf("Neplatny targetNote(%d) alebo targetOctave(%d).\n", targetNote, targetOctave);
    return 0;
  }
  int lastSteps = (hand.currentOctave - 1) * stepsPerOctave + hand.currentNote * stepsPerNote;
  int steps = (targetOctave - 1) * stepsPerOctave + targetNote * stepsPerNote;
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
  int holdTime = wait - (rezerva + hand.timeFromMoving);
  if (holdTime < 0) {
    Serial.printf("Pozor, negativny holdTime(%d ms). Ides na 50 ms.\n", holdTime);
    holdTime = 50;
  }
  while (millis() - hand.lastTime <= holdTime) {
    for (int i = 0; i < 3; i++){
      if (notes[i] != -1 && notes[i] <= numServos){
        hand.pca9685->setPWM(notes[i], 0, hand.pressValue); 
      }
    }
  }
  hand.lastTime = millis();
  while (millis() - hand.lastTime <= rezerva ) {
    for (int i = 0; i < 3; i++){
      if (notes[i] != -1 && notes[i] <= numServos){
        hand.pca9685->setPWM(notes[i], 0, hand.releaseValue); 
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

int havasiFreedomRight1[][6] = {
    //prvy takt
    {A, 1, cel, NIC, NIC, NIC},
};
int havasiFreedomRight2[][6] = {
    // druhy takt
    {A, 1, cel, NIC, NIC, NIC},
};
int havasiFreedomRight3[][6] = {
    //treti takt
    {A, 1, cel, NIC, NIC, NIC},
};
int havasiFreedomRight4[][6] = {
    //stvrty takt
    {A, 1, cel, NIC, NIC, NIC},
};
int havasiFreedomRight5[][6] = {
    //piaty takt
    {A, 1, osm, SERVO1, SERVO3, SERVO5},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, osm, SERVO1, SERVO3, SERVO5},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, stv, SERVO1, SERVO3, NIC},
    {A, 1, sest, SERVO2, NIC, NIC},
    {A, 1, sest, SERVO3, NIC, NIC},
    {A, 1, osm, SERVO4, NIC, NIC},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, sest, SERVO5, NIC, NIC},
};
int havasiFreedomRight6[][6] = {
    //prvy takt
    {A, 1, stv, SERVO1, SERVO3, NIC},
    {A, 1, osm, NIC, NIC, NIC},
    {A, 1, stv, NIC, NIC, NIC},
    {A, 1, sest, SERVO2, NIC, NIC},
    {A, 1, sest, SERVO3, NIC, NIC},
    {A, 1, osm, SERVO2, SERVO4, NIC},
    {A, 1, osm, SERVO3, SERVO5, NIC},
};
int havasiFreedomRight7[][6] = {
    //druhy takt
    {A, 1, osm, SERVO1, SERVO3, SERVO5},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, osm, SERVO1, SERVO3, NIC},
    {A, 1, stv, NIC, NIC, NIC},
    {A, 1, sest, SERVO1, NIC, NIC},
    {A, 1, sest, SERVO2, NIC, NIC},
    {A, 1, osm, SERVO1, SERVO3, NIC},
    {A, 1, osm, SERVO2, SERVO4},
};
//druhy takt
int havasiFreedomRight8[][6] = {
    {A, 1, osm, SERVO1, SERVO3, SERVO5},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, osm, SERVO1, SERVO3},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, stv, NIC, NIC, NIC},
    {A, 1, sest, SERVO2, NIC, NIC},
    {A, 1, sest, SERVO3, NIC, NIC},
    {A, 1, osm, SERVO4, NIC, NIC},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, sest, SERVO5, NIC, NIC},
};
int havasiFreedomRight9[][6] = {
    //treti takt
    {A, 1, stv, SERVO1, SERVO3, NIC},
    {A, 1, osm, NIC, NIC, NIC},
    {A, 1, stv, NIC, NIC, NIC},
    {A, 1, sest, SERVO1, NIC, NIC},
    {A, 1, sest, SERVO2, NIC, NIC},
    {A, 1, osm, SERVO1, SERVO3, NIC},
    {A, 1, osm, SERVO1, SERVO4, NIC},
};
int havasiFreedomRight10[][6] = {
    //stvrty takt
    {A, 1, osm, SERVO1, SERVO3, SERVO5},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, osm, SERVO1, SERVO3, SERVO5},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, stv, NIC, NIC, NIC},
    {A, 1, sest, SERVO2, NIC, NIC},
    {A, 1, sest, SERVO3, NIC, NIC},
    {A, 1, osm, SERVO2, SERVO4, NIC},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, sest, SERVO3, SERVO5, NIC},
};
int havasiFreedomRight11[][6] = {
    //prvy takt
    {A, 1, stv, SERVO1, SERVO3, NIC},
    {A, 1, stv, NIC, NIC, NIC},
    {A, 1, sest, SERVO2, NIC, NIC},
    {A, 1, sest, SERVO3, NIC, NIC},
    {A, 1, osm, SERVO2, SERVO4, NIC},
    {A, 1, osm, SERVO3, SERVO5, NIC},
};
int havasiFreedomRight12[][6] = {
    //druhy takt
    {A, 1, osm, SERVO1, SERVO3, SERVO5},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, osm, SERVO1, SERVO3, NIC},
    {A, 1, stv, NIC, NIC, NIC},
    {A, 1, sest, SERVO1, NIC, NIC},
    {A, 1, sest, SERVO2, NIC, NIC},
    {A, 1, osm, SERVO1, SERVO3, NIC},
    {A, 1, osm, SERVO2, SERVO4},
};
int havasiFreedomRight13[][6] = {
    // treti takt, tu je toten krizik sprosty
    {G, 1, osm, SERVO1, SERVO6, NIC },
    {G, 1, sest, NIC, NIC, NIC},
    {G, 1, osm, SERVO1, SERVO7, NIC },
    {G, 1, osm, SERVO1, SERVO7, NIC },
    {G, 1, sest, NIC, NIC, NIC},
    {G, 1, stv, SERVO1, SERVO6, NIC },
    {G, 1, stv, SERVO1, SERVO6, NIC },
    //aaa tu sa to uz opakovat bude, nemam nervy pisat toto, a asi to nebude 
};
// freedon left hand
int havasiFreedomLeft1[][6] = {
    //prvy takt
    {A, 1, osm, SERVO1, NIC, NIC},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, osm, NIC, SERVO8, NIC},
    {A, 1, sest, NIC, NIC, NIC},
    {A, 1, osm, NIC, SERVO8, NIC},
    {A, 1, stv, NIC, SERVO8, NIC},
    {A, 1, stv, NIC, SERVO8, NIC},
};

int havasiFreedomLeft5[][6] = {
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
};
int havasiFreedomLeft9[][6] = {
    {F, 1, osm, SERVO1, SERVO8, NIC},
    {F, 1, sest, NIC, NIC, NIC},
    {F, 1, osm, SERVO1, SERVO8, NIC},
    {F, 1, sest, NIC, NIC, NIC},
    {F, 1, osm, SERVO1, SERVO8, NIC},
    {F, 1, sest, NIC, NIC, NIC},
    {F, 1, osm, SERVO1, SERVO8, NIC},
    {F, 1, sest, NIC, NIC, NIC},
    {F, 1, stv, SERVO1, SERVO8, NIC},
};
int havasiFreedomLeft12[][6] = {
    {E, 1, osm, SERVO1, SERVO8, NIC},
    {E, 1, sest, NIC, NIC, NIC},
    {E, 1, osm, SERVO1, SERVO8, NIC},
    {E, 1, sest, NIC, NIC, NIC},
    {E, 1, osm, SERVO1, SERVO8, NIC},
    {E, 1, sest, NIC, NIC, NIC},
    {E, 1, osm, SERVO1, SERVO8, NIC},
    {E, 1, sest, NIC, NIC, NIC},
    {E, 1, stv, SERVO1, SERVO8, NIC},
};  
// fireball zatial nie je zahrnuta
int fireballRight1[][6] = {
  // prvy a treti takt
  {G, 1, stv, SERVO1, NIC, NIC},
  {G, 1, osm, SERVO8, NIC, NIC},
  {G, 1, osm, SERVO8, NIC, NIC},
  {G, 1, sest, SERVO7, NIC, NIC},
  {G, 1, osm, SERVO7, NIC, NIC},
  {G, 1, sest, SERVO7, NIC, NIC},
  {G, 1, stv, NIC, NIC, NIC},
};
int fireballRight3[][6] = {
  // druhy a stvrty takt
  {G, 1, osm, NIC, NIC, NIC},
  {G, 1, osm, SERVO1, NIC, NIC},
  {G, 1, osm, SERVO8, NIC, NIC},
  {G, 1, osm, SERVO8, NIC, NIC},
  {G, 1, sest, SERVO7, NIC, NIC},
  {G, 1, osm, SERVO7, NIC, NIC},
  {G, 1, sest, SERVO7, NIC, NIC},
  {C, 2, stv, NIC, NIC, NIC},
};
  //piaty a siedny takt
int fireballRight5[][6] = {
  {C, 2, stv, SERVO4, NIC, NIC},
  {C, 2, osm, SERVO5, NIC, NIC},
  {C, 2, osm, SERVO3, NIC, NIC},
  {C, 2, sest, SERVO4, NIC, NIC},
  {C, 2, osm, SERVO4, NIC, NIC},
  {C, 2, sest, SERVO1, NIC, NIC},
  {C, 2, stv, NIC, NIC, NIC},
};
int fireballRight6[][6] = {
  //sesty takt
  {C, 2, osm, NIC, NIC, NIC},
  {C, 2, osm, SERVO4, NIC, NIC},
  {C, 2, osm, SERVO5, NIC, NIC},
  {C, 2, osm, SERVO3, NIC, NIC},
  {C, 2, sest, SERVO4, NIC, NIC},
  {C, 2, osm, SERVO4, NIC, NIC},
  {C, 2, sest, SERVO1, NIC, NIC},
  {C, 2, stv, NIC, NIC, NIC},
};
int fireballRight8[][6] = {
  //osmy takt
  {C, 2, osm, NIC, NIC, NIC},
  {C, 2, osm, SERVO4, NIC, NIC},
  {C, 2, osm, SERVO5, NIC, NIC},
  {C, 2, osm, SERVO3, NIC, NIC},
  {C, 2, sest, SERVO4, NIC, NIC},
  {C, 2, osm, SERVO4, NIC, NIC},
  {C, 2, sest, NIC, NIC, NIC},
  {C, 2, stv, SERVO8, NIC, NIC},
};
int fireballLeft[][6] = {
  // kazdy jeden
  {F, 1, stv, SERVO1, NIC, NIC},
  {F, 1, osm, SERVO5, NIC, NIC},
  {F, 1, osm, SERVO7, NIC, NIC},
  {F, 1, stv, SERVO8, NIC, NIC},
  {F, 1, osm, SERVO7, NIC, NIC},
  {F, 1, osm, SERVO5, NIC, NIC},
};

void loop() {
  xTaskCreatePinnedToCore([] (void *) {
    playMelody(leftHand, havasiFreedomLeft1, sizeof(havasiFreedomLeft1) / sizeof(havasiFreedomLeft1[0]));
    vTaskDelete(NULL);
  }, "LeftHandTask", 4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore([] (void *) {
      playMelody(rightHand, havasiFreedomRight1, sizeof(havasiFreedomRight1) / sizeof(havasiFreedomRight1[0]));
    vTaskDelete(NULL);
  }, "RightHandTask", 4096, NULL, 1, NULL, 1); 
}
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  if(myData == 1){
    xTaskCreatePinnedToCore([] (void *) {
      playMelody(leftHand, havasiFreedomLeft1, sizeof(havasiFreedomLeft1) / sizeof(havasiFreedomLeft1[0]));
      vTaskDelete(NULL);
    }, "LeftHandTask", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore([] (void *) {
       playMelody(rightHand, havasiFreedomRight1, sizeof(havasiFreedomRight1) / sizeof(havasiFreedomRight1[0]));
      vTaskDelete(NULL);
    }, "RightHandTask", 4096, NULL, 1, NULL, 1); 
  }
  if(myData == 2) {
    xTaskCreatePinnedToCore([] (void *) {
        playMelody(leftHand, havasiFreedomLeft1, sizeof(havasiFreedomLeft1) / sizeof(havasiFreedomLeft1[0]));
        vTaskDelete(NULL);
    }, "LeftHandTask", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore([] (void *) {
        playMelody(rightHand, havasiFreedomRight2, sizeof(havasiFreedomRight2) / sizeof(havasiFreedomRight2[0]));
        vTaskDelete(NULL);
    }, "RightHandTask", 4096, NULL, 1, NULL, 1); 
  }
  if(myData == 3) {
    xTaskCreatePinnedToCore([] (void *) {
        playMelody(leftHand, havasiFreedomLeft1, sizeof(havasiFreedomLeft1) / sizeof(havasiFreedomLeft1[0]));
        vTaskDelete(NULL);
    }, "LeftHandTask", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore([] (void *) {
        playMelody(rightHand, havasiFreedomRight3, sizeof(havasiFreedomRight3) / sizeof(havasiFreedomRight3[0]));
        vTaskDelete(NULL);
    }, "RightHandTask", 4096, NULL, 1, NULL, 1); 
  }
  if(myData == 4) {
    xTaskCreatePinnedToCore([] (void *) {
        playMelody(leftHand, havasiFreedomLeft1, sizeof(havasiFreedomLeft1) / sizeof(havasiFreedomLeft1[0]));
        vTaskDelete(NULL);
    }, "LeftHandTask", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore([] (void *) {
        playMelody(rightHand, havasiFreedomRight4, sizeof(havasiFreedomRight4) / sizeof(havasiFreedomRight4[0]));
        vTaskDelete(NULL);
    }, "RightHandTask", 4096, NULL, 1, NULL, 1); 
  }
  if(myData == 5) {
    xTaskCreatePinnedToCore([] (void *) {
        playMelody(leftHand, havasiFreedomLeft5, sizeof(havasiFreedomLeft5) / sizeof(havasiFreedomLeft5[0]));
        vTaskDelete(NULL);
    }, "LeftHandTask", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore([] (void *) {
        playMelody(rightHand, havasiFreedomRight5, sizeof(havasiFreedomRight5) / sizeof(havasiFreedomRight5[0]));
        vTaskDelete(NULL);
    }, "RightHandTask", 4096, NULL, 1, NULL, 1); 
  }
  if(myData == 6) {
    xTaskCreatePinnedToCore([] (void *) {
        playMelody(leftHand, havasiFreedomLeft5, sizeof(havasiFreedomLeft5) / sizeof(havasiFreedomLeft5[0]));
        vTaskDelete(NULL);
    }, "LeftHandTask", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore([] (void *) {
        playMelody(rightHand, havasiFreedomRight6, sizeof(havasiFreedomRight6) / sizeof(havasiFreedomRight6[0]));
        vTaskDelete(NULL);
    }, "RightHandTask", 4096, NULL, 1, NULL, 1); 
  }
  if(myData == 7) {
    xTaskCreatePinnedToCore([] (void *) {
        playMelody(leftHand, havasiFreedomLeft5, sizeof(havasiFreedomLeft5) / sizeof(havasiFreedomLeft5[0]));
        vTaskDelete(NULL);
    }, "LeftHandTask", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore([] (void *) {
        playMelody(rightHand, havasiFreedomRight7, sizeof(havasiFreedomRight7) / sizeof(havasiFreedomRight7[0]));
        vTaskDelete(NULL);
    }, "RightHandTask", 4096, NULL, 1, NULL, 1); 
  }
  if(myData == 8) {
    xTaskCreatePinnedToCore([] (void *) {
        playMelody(leftHand, havasiFreedomLeft5, sizeof(havasiFreedomLeft5) / sizeof(havasiFreedomLeft5[0]));
        vTaskDelete(NULL);
    }, "LeftHandTask", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore([] (void *) {
        playMelody(rightHand, havasiFreedomRight8, sizeof(havasiFreedomRight8) / sizeof(havasiFreedomRight8[0]));
        vTaskDelete(NULL);
    }, "RightHandTask", 4096, NULL, 1, NULL, 1); 
  }
  if(myData == 9) {
    xTaskCreatePinnedToCore([] (void *) {
        playMelody(leftHand, havasiFreedomLeft9, sizeof(havasiFreedomLeft9) / sizeof(havasiFreedomLeft9[0]));
        vTaskDelete(NULL);
    }, "LeftHandTask", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore([] (void *) {
        playMelody(rightHand, havasiFreedomRight9, sizeof(havasiFreedomRight9) / sizeof(havasiFreedomRight9[0]));
        vTaskDelete(NULL);
    }, "RightHandTask", 4096, NULL, 1, NULL, 1); 
  }
  if(myData == 10) {
    xTaskCreatePinnedToCore([] (void *) {
        playMelody(leftHand, havasiFreedomLeft9, sizeof(havasiFreedomLeft9) / sizeof(havasiFreedomLeft9[0]));
        vTaskDelete(NULL);
    }, "LeftHandTask", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore([] (void *) {
        playMelody(rightHand, havasiFreedomRight10, sizeof(havasiFreedomRight10) / sizeof(havasiFreedomRight10[0]));
        vTaskDelete(NULL);
    }, "RightHandTask", 4096, NULL, 1, NULL, 1); 
  }
  if(myData == 11) {
    xTaskCreatePinnedToCore([] (void *) {
        playMelody(leftHand, havasiFreedomLeft9, sizeof(havasiFreedomLeft9) / sizeof(havasiFreedomLeft9[0]));
        vTaskDelete(NULL);
    }, "LeftHandTask", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore([] (void *) {
        playMelody(rightHand, havasiFreedomRight11, sizeof(havasiFreedomRight11) / sizeof(havasiFreedomRight11[0]));
        vTaskDelete(NULL);
    }, "RightHandTask", 4096, NULL, 1, NULL, 1); 
  }
  if(myData == 12) {
    xTaskCreatePinnedToCore([] (void *) {
        playMelody(leftHand, havasiFreedomLeft12, sizeof(havasiFreedomLeft12) / sizeof(havasiFreedomLeft12[0]));
        vTaskDelete(NULL);
    }, "LeftHandTask", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore([] (void *) {
        playMelody(rightHand, havasiFreedomRight12, sizeof(havasiFreedomRight12) / sizeof(havasiFreedomRight12[0]));
        vTaskDelete(NULL);
    }, "RightHandTask", 4096, NULL, 1, NULL, 1); 
  }
  if(myData == 20 || myData == 22){
    xTaskCreatePinnedToCore([] (void *) {
        playMelody(leftHand, fireballLeft, sizeof(fireballLeft) / sizeof(fireballLeft[0]));
        vTaskDelete(NULL);
    }, "LeftHandTask", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore([] (void *) {
        playMelody(rightHand, fireballRight1, sizeof(fireballRight1) / sizeof(fireballRight1[0]));
        vTaskDelete(NULL);
    }, "RightHandTask", 4096, NULL, 1, NULL, 1); 
  }
  if(myData == 21 || myData == 23){
    xTaskCreatePinnedToCore([] (void *) {
        playMelody(leftHand, fireballLeft, sizeof(fireballLeft) / sizeof(fireballLeft[0]));
        vTaskDelete(NULL);
    }, "LeftHandTask", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore([] (void *) {
        playMelody(rightHand, fireballRight3, sizeof(fireballRight3) / sizeof(fireballRight3[0]));
        vTaskDelete(NULL);
    }, "RightHandTask", 4096, NULL, 1, NULL, 1); 
  }
  if( myData == 24 || myData == 26){
    xTaskCreatePinnedToCore([] (void *) {
        playMelody(leftHand, fireballLeft, sizeof(fireballLeft) / sizeof(fireballLeft[0]));
        vTaskDelete(NULL);
    }, "LeftHandTask", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore([] (void *) {
        playMelody(rightHand, fireballRight5, sizeof(fireballRight5) / sizeof(fireballRight5[0]));
        vTaskDelete(NULL);
    }, "RightHandTask", 4096, NULL, 1, NULL, 1);
  }
  if(myData == 25) {
    xTaskCreatePinnedToCore([] (void *) {
        playMelody(leftHand, fireballLeft, sizeof(fireballLeft) / sizeof(fireballLeft[0]));
        vTaskDelete(NULL);
    }, "LeftHandTask", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore([] (void *) {
        playMelody(rightHand, fireballRight6, sizeof(fireballRight6) / sizeof(fireballRight6[0]));
        vTaskDelete(NULL);
    }, "RightHandTask", 4096, NULL, 1, NULL, 1);
  }
  if (myData == 28){
    xTaskCreatePinnedToCore([] (void *) {
        playMelody(leftHand, fireballLeft, sizeof(fireballLeft) / sizeof(fireballLeft[0]));
        vTaskDelete(NULL);
    }, "LeftHandTask", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore([] (void *) {
        playMelody(rightHand, fireballRight8, sizeof(fireballRight8) / sizeof(fireballRight8[0]));
        vTaskDelete(NULL);
    }, "RightHandTask", 4096, NULL, 1, NULL, 1);
  }
  
}