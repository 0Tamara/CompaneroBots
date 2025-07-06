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


int havasiFreedomRight1[][4] = {
    {A, 1, sest, 0b10101010}, // osm
    {A, 1, sest, 0b00000000},
    {A, 1, sest, 0b00000000}, 
    {A, 1, sest, 0b10101010}, // osm
    {A, 1, sest, 0b00000000},
    {A, 1, sest, 0b00000000}, // stv pomlcka
    {A, 1, sest, 0b10100000},
    {A, 1, sest, 0b00000000},
    {A, 1, sest, 0b00000000},
    {A, 1, sest, 0b10000000},
    {A, 1, sest, 0b01000000},
    {A, 1, sest, 0b00100000},
    {A, 1, sest, 0b01010000},
    {A, 1, sest, 0b00000000},
    {A, 1, sest, 0b00000000},
    {A, 1, sest, 0b00001000},
  };
  int havasiFreedomLeft1[][4] = {
    {A, 1, sest, 0b10000000}, //osm
    {A, 1, sest, 0b00000000},
    {A, 1, sest, 0b00000000}, // sest pomlcka
    {A, 1, sest, 0b00000001}, //osm
    {A, 1, sest, 0b00000000}, 
    {A, 1, sest, 0b00000000}, //sest pomlcka
    {A, 1, sest, 0b10000000}, //osm
    {A, 1, sest, 0b00000000},
    {A, 1, sest, 0b10000000}, //stv
    {A, 1, sest, 0b00000000},
    {A, 1, sest, 0b00000000},
    {A, 1, sest, 0b00000000}, 
    {A, 1, sest, 0b10000000}, //stv
    {A, 1, sest, 0b00000000},
    {A, 1, sest, 0b00000000},
    {A, 1, sest, 0b00000000},
  };

int havasiFreedomRight5[] = {
  0b10101000, // osm
  0b00000000,
  0b00000000, // sest (no servo)
  0b10101000, // osm
  0b00000000,
  0b00000000, // sest (no servo)
  0b10100000, // stv
  0b00000000,
  0b00000000,
  0b00000000,
  0b01000000, // sest
  0b00100000, // sest
  0b00010000, // osm
  0b00000000,
  0b00000000, // sest (no servo)
  0b00001000  // sest
};

int havasiFreedomRight6Bits[] = {
  0b10100000, // line 1, stv
  0b00000000, // wait 1
  0b00000000, // wait 2
  0b00000000, // wait 3
  0b00000000, // line 2, osm
  0b00000000, // wait 1
  0b00000000, // line 3, stv
  0b00000000, // wait 1
  0b00000000, // wait 2
  0b00000000, // wait 3
  0b01000000, // line 4 sest
  0b00100000, // line 5 sest
  0b01010000, // line 6 osm
  0b00000000, // wait 1
  0b00101000, // line 7 osm
  0b00000000  // wait 1
};

int havasiFreedomRight7[][6] = {
    //druhy takt
    {A, 2, osm, SERVO1, SERVO3, SERVO5},
    {A, 2, sest, NIC, NIC, NIC},
    {A, 2, osm, SERVO1, SERVO3, NIC},
    {A, 2, stv, NIC, NIC, NIC},
    {A, 2, sest, SERVO1, NIC, NIC},
    {A, 2, sest, SERVO2, NIC, NIC},
    {A, 2, osm, SERVO1, SERVO3, NIC},
    {A, 2, osm, SERVO2, SERVO4},
};
//druhy takt
int havasiFreedomRight8[][6] = {
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
};
int havasiFreedomRight9[][6] = {
    //treti takt
    {A, 2, stv, SERVO1, SERVO3, NIC},
    {A, 2, osm, NIC, NIC, NIC},
    {A, 2, stv, NIC, NIC, NIC},
    {A, 2, sest, SERVO1, NIC, NIC},
    {A, 2, sest, SERVO2, NIC, NIC},
    {A, 2, osm, SERVO1, SERVO3, NIC},
    {A, 2, osm, SERVO1, SERVO4, NIC},
};
int havasiFreedomRight10[][6] = {
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
};
int havasiFreedomRight11[][6] = {
    //prvy takt
    {A, 2, stv, SERVO1, SERVO3, NIC},
    {A, 2, stv, NIC, NIC, NIC},
    {A, 2, sest, SERVO2, NIC, NIC},
    {A, 2, sest, SERVO3, NIC, NIC},
    {A, 2, osm, SERVO2, SERVO4, NIC},
    {A, 2, osm, SERVO3, SERVO5, NIC},
};
int havasiFreedomRight12[][6] = {
    //druhy takt
    {A, 2, osm, SERVO1, SERVO3, SERVO5},
    {A, 2, sest, NIC, NIC, NIC},
    {A, 2, osm, SERVO1, SERVO3, NIC},
    {A, 2, stv, NIC, NIC, NIC},
    {A, 2, sest, SERVO1, NIC, NIC},
    {A, 2, sest, SERVO2, NIC, NIC},
    {A, 2, osm, SERVO1, SERVO3, NIC},
    {A, 2, osm, SERVO2, SERVO4},
};
int havasiFreedomRight13[][6] = {
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
// freedon left hand

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




void havasiFreedom1(){
  
  int lenght2 = sizeof(havasiFreedomLeft1) / sizeof(havasiFreedomLeft1[0]);
  int lenght = sizeof(havasiFreedomRight1) / sizeof(havasiFreedomRight1[0]);
  
  int targetNote = havasiFreedomRight1[0][0];
  int targetOctave = havasiFreedomRight1[0][1];
  int targetNote2 = havasiFreedomLeft1[0][0];
  int targetOctave2 = havasiFreedomLeft1[0][1];
  int steps = stepsPerNote * targetNote + stepsPerNote * (targetOctave - 1);
  int steps2 = stepsPerNote * targetNote2 + stepsPerNote * (targetOctave2 - 1);
  rightHand.stepper->moveTo(steps);
  leftHand.stepper->moveTo(steps2);
  while (rightHand.stepper->isRunning() || leftHand.stepper->isRunning()){};

  for (int i = 0; i < lenght; i++)
  {
    unsigned long start = millis();
    int wait = havasiFreedomRight1[i][2];
    byte notes = havasiFreedomRight1[i][3];

    int wait2 = havasiFreedomLeft1[i][2];
    byte notes2 = havasiFreedomLeft1[i][3];
    
    for (int j = 0; j < 8; j++)
    {
      if (notes & 1<<j)
      {
        pca9685right.setPWM(j+8, 0, rightHand.pressValue);
      }
      if (notes2 & 1<<j)
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
  //xTaskCreatePinnedToCore([] (void *) {
  havasiFreedom1();
  //  vTaskDelete(NULL);
  //}, "LeftHandTask", 4096, NULL, 1, NULL, 0);
  delay(500);
}

/*
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
  const char* handName = (hand.pca9685 == &pca9685right) ? "RIGHT" : "LEFT";
  Serial.printf("[%s] playNote: Zacina, nota %d, oktava %d, wait %d ms, serva: %d, %d, %d\n", 
                handName, targetNote, targetOctave, wait, note1, note2, note3);
  unsigned long start = millis();
  
  hand.timeFromMoving = moveToNote(hand, targetNote, targetOctave);
  int notes[3] = {note1, note2, note3};
  hand.lastTime = millis();
  int holdTime = wait - (rezerva + hand.timeFromMoving);
  if (holdTime < 0) {
    Serial.printf("[%s] Pozor, negativny holdTime(%d ms). Ides na 50 ms.\n", handName, holdTime);
    holdTime = 50;
  }
    // Stlač servá
  for (int i = 0; i < 3; i++) {
    if (notes[i] != NIC && notes[i] >= 0 && notes[i] < numServos) {
      Serial.printf("[%s] Stlacanie serva %d na hodnote %d\n", handName, notes[i], hand.pressValue);
      hand.pca9685.setPWM(notes[i], 0, hand.pressValue);
    }
  }
  for (int i = 0; i < 3; i++) {
    if (notes[i] != NIC && notes[i] >= 0 && notes[i] < numServos) {
      Serial.printf("[%s] Stlacanie serva %d na hodnote %d\n", handName, notes[i], hand.pressValue);
      hand.pca9685.setPWM(notes[i], 0, hand.releaseValue);
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
  
}*/