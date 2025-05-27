#include <ESP32Servo.h>

#define numServos 8
#define rezerva 20
#define stepsPerNote 25
#define stepsPerOctave 200
unsigned long lastTime;

//na pamatanie aktualnej oktavy
int currentOctave = 0;
int currentNote = 0;
int noteCount = 0;
int speedDelay = 0;
unsigned long timeFromMoving = 0;
//serva
Servo servos[numServos];
int leftServoPins[numServos] = { 2, 3, 4, 5, 6, 7, 8, 9 };  
//krok motor
int stepPin = 10;
int dirPin = 11;
// casy
int time = 2000;
int osm = time / 8;
int stv = time / 4;
int pol = time / 2;
int cel = time;

enum moveNotes { C=0, D=1, E=2, F=3, G=4, A=5, H=6 };
enum actualNotes { NIC = -1, SERVO1 = 0, SERVO2 = 1, SERVO3 = 2, SERVO4 = 3, SERVO5 = 4, SERVO6 = 5, SERVO7 = 6, SERVO8 = 7 };

void setup() {
  for (int i = 0; i < numServos; i++) {
    servos[i].attach(leftServoPins[i]);
  }
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
}

// Funkcia na presun na pozadovany ton a oktavu
unsigned long moveToNote(int targetNote, int targetOctave) {
  lastTime = millis();
  int direction = (targetOctave * stepsPerOctave + targetNote * stepsPerNote) - 
                    (currentOctave * stepsPerOctave + currentNote * stepsPerNote);

  if (direction == 0) return 0;

  digitalWrite(dirPin, direction > 0 ? HIGH : LOW);
  int steps = abs(direction);
  speedDelay = map(steps, 0, 800, 200, 30);

  for (int i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(speedDelay);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(speedDelay);
  }

  currentOctave = targetOctave;
  currentNote = targetNote;
  return millis() - lastTime;
}
// Funkcia na prehratie konkretnej noty
void playNote( int targetNote, int targetOctave, int wait, int note1, int note2, int note3) {
  timeFromMoving = moveToNote(targetNote, targetOctave);
  int notes[3] = {note1, note2, note3};
  lastTime = millis();
  noteCount = 0;
  for(int i = 0; i < 3; i++){
    if(notes[i] != -1 && notes[i] < numServos) {
        noteCount +=1;
    }
  }
  int holdTime = wait - (rezerva * noteCount + timeFromMoving);
  if (holdTime < 0) {
    holdTime = 0;
  }
  while (millis() - lastTime <= holdTime) {
    for (int i = 0; i < 3; i++){
      if (notes[i] != -1 && notes[i] < numServos){
        servos[notes[i]].write(90);
      }
    }
  }
  lastTime = millis();
  while (millis() - lastTime <= rezerva * noteCount) {
    for (int i = 0; i < 3; i++){
      if (notes[i] != -1 && notes[i] < numServos){
        servos[notes[i]].write(0);
      }
    }
  }
}
// Funkcia na prehratie melodie
void playMelody(int melody[][6], int length) {
  for (int i = 0; i < length; i++) {
    int targetNote = melody[i][0];
    int targetOctave = melody[i][1];
    int wait = melody[i][2];
    int note1 = melody[i][3];
    int note2 = melody[i][4];
    int note3 = melody[i][5];
    playNote(targetNote, targetOctave, wait, note1, note2, note3);
  }
}
int melody1[][6] = {
    {D, 5, osm, NIC, NIC, SERVO2},
    {C, 4, stv, SERVO2, NIC, SERVO5}

};
void loop() {
    playMelody(melody1, sizeof(melody1) / sizeof(melody1[0]));
}