#include <ESP32Servo.h>
#include <Arduino.h>
#include<AccelStepper.h>

#define numServos 8
#define rezerva 20
#define stepsPerNote 25
#define stepsPerOctave 200
unsigned long lastTime;

// casy
int time = 2000;
int osm = time / 8;
int stv = time / 4;
int pol = time / 2;
int cel = time;
const int leftHandStepPin = 10;
const int leftHandDirPin = 11;
const int rightHandStepPin = 20;
const int rightHandDirPin = 21;

AccelStepper stepperLeft(AccelStepper::DRIVER, leftHandStepPin, leftHandDirPin);
AccelStepper stepperRight(AccelStepper::DRIVER, rightHandStepPin, rightHandDirPin);

enum moveNotes { C=0, D=1, E=2, F=3, G=4, A=5, H=6 };
enum actualServos { NIC = -1, SERVO1 = 0, SERVO2 = 1, SERVO3 = 2, SERVO4 = 3, SERVO5 = 4, SERVO6 = 5, SERVO7 = 6, SERVO8 = 7 };

struct Hand
{
  Servo servos[numServos];
  int servoPins[numServos];
  //int stepPin;
  //int dirPin;
  int currentOctave;
  int currentNote;
  AccelStepper& stepper;
  unsigned long timeFromMoving;
  unsigned long lastTime;
};

Hand leftHand = {
  .servoPins = { 2, 3, 4, 5, 6, 7, 8, 9 },
  .currentOctave = 0,
  .currentNote = 0,
  .stepper = stepperLeft,
};
Hand rightHand = {
  .servoPins = { 12, 13, 14, 15, 16, 17, 18, 19 },
  .currentOctave = 0,
  .currentNote = 0,
  .stepper = stepperRight,
};


void setup() {
  for (int i = 0; i < numServos; i++) {
    leftHand.servos[i].attach(leftHand.servoPins[i]);
    rightHand.servos[i].attach(rightHand.servoPins[i]); 
  
    leftHand.servos[i].write(0);
    rightHand.servos[i].write(0);
  }
  stepperLeft.setMaxSpeed(1000);
  stepperLeft.setAcceleration(500);
  stepperLeft.setCurrentPosition(0);

  stepperRight.setMaxSpeed(1000);
  stepperRight.setAcceleration(500);
  stepperRight.setCurrentPosition(0);
  
}

unsigned long moveToNote(Hand& hand, int targetNote, int targetOctave) {
  unsigned long start = millis();
  int steps = (targetOctave * stepsPerOctave + targetNote * stepsPerNote) - 
                    (hand.currentOctave * stepsPerOctave + hand.currentNote * stepsPerNote);

  if (steps == 0) return 0;
  hand.stepper.move(steps);
  while (hand.stepper.distanceToGo() != 0) {
    hand.stepper.run();
  }
  hand.currentOctave = targetOctave;
  hand.currentNote = targetNote;
  return millis() - start;
}
// Funkcia na prehratie konkretnej noty
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
    holdTime = 0;
  }
  while (millis() - hand.lastTime <= holdTime) {
    for (int i = 0; i < 3; i++){
      if (notes[i] != -1 && notes[i] < numServos){
        hand.servos[notes[i]].write(90);
      }
    }
  }
  hand.lastTime = millis();
  while (millis() - hand.lastTime <= rezerva * noteCount) {
    for (int i = 0; i < 3; i++){
      if (notes[i] != -1 && notes[i] < numServos){
        hand.servos[notes[i]].write(0);
      }
    }
  }
}
// Funkcia na prehratie melodie
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
    {D, 5, osm, NIC, NIC, SERVO2},
    {C, 4, stv, SERVO2, NIC, SERVO5}

};
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
    }, "RightHandTask", 4096, NULL, 1, NULL, 1);
    while(true); //konec programu
}