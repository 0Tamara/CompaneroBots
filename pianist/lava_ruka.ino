#include <ESP32Servo.h>

#define numServos 8
#define rezerva 100
#define stepsPerNote 25
#define stepsPerOctave 200
unsigned long lastTime;

//na pamatanie aktualnej oktavy
int currentOctave = 0;
int currentNote = 0;
//serva
Servo servos[numServos];
int leftServoPins[numServos] = { 2, 3, 4, 5, 6, 7, 8, 9 };  
//krok motor
int stepPin = 10;
int dirPin = 11;
// casy
int time = 1000;
int osm = time / 8;
int stv = time / 4;
int pol = time / 2;
int cel = time;

void setup() {
  for (int i = 0; i < numServos; i++) {
    servos[i].attach(leftServoPins[i]);
  }
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
}

void moveToOctave(int targetOctave) {
  int direction = targetOctave - currentOctave;
  if (direction == 0) return;

  digitalWrite(dirPin, direction > 0 ? HIGH : LOW);
  int steps = abs(direction) * stepsPerOctave;

  for (int i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(800);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(800);
  }

  currentOctave = targetOctave;
}

void moveToNote(int targetNote, int targetOctave) {
  int direction = (targetOctave * stepsPerOctave + targetNote * stepsPerNote) - (currentOctave * stepsPerOctave + currentNote * stepsPerNote);

  if (direction == 0) return;

  digitalWrite(dirPin, direction > 0 ? HIGH : LOW);
  int steps = abs(direction);

  for (int i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(800);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(800);
  }

  currentOctave = targetOctave;
  currentNote = targetNote;
}

//tu mozem spravit aj to, ze bude rovnaky kod ako na pravu ruku, ale vlastne mal by som noteIndex1 - 8, a posuval by som sa priamo na notu
void playDualNote(int note, int octave, int wait) {
  moveToNote(note, octave);
  lastTime = millis();
  while (millis() - lastTime <= (wait - rezerva)) {
    servos[0].write(90);
    servos[7].write(90);
  }
  lastTime = millis();
  while (millis() - lastTime <= rezerva) {
    servos[0].write(0);
    servos[7].write(0);
  }
}
void playNote(int noteIndex, int octave, int wait) {
  moveToOctave(octave);
  lastTime = millis();
  while (millis() - lastTime <= (wait - rezerva)) {
    servos[noteIndex].write(90);
  }
  lastTime = millis();
  while (millis() - lastTime <= rezerva) {
    servos[noteIndex].write(0);
  }
}
void playDualMelody(int dualMelody[][3], int length) {
  for (int i = 0; i < length; i++) {
    int octave = dualMelody[i][0];
    int wait = dualMelody[i][1];
    int note = dualMelody[i][2];
    playDualNote(note, octave, wait);
  }
}

void playMelody(int melody[][3], int length) {
  for (int i = 0; i < length; i++) {
    int octave = melody[i][0];
    int wait = melody[i][1];
    int note = melody[i][2];
    playNote(note, octave, wait);
  }
}
int dualMelody1[][3] = {
    { 1, stv, 2 },
    { 5, osm, 3 },
    { 1, stv, 4 },
    { 5, osm, 5 },
    { 1, stv, 6 },
    { 5, osm, 7 },
    { 1, stv, 5 },
    { 5, osm, 4 },
    { 0, osm, 0} // aby potom nemal problemy s dalsou melodiou
};

int melody1[][3] = {
  //h
    { 1, stv, 0 },
    { 1, osm, 1 },
    { 1, stv, 2 },
    { 1, osm, 3 },
    { 1, stv, 4 },
    { 1, osm, 5 },
    { 1, stv, 6 },
    { 1, osm, 7 }
  //d
    { 1, stv, 7 },
    { 1, osm, 6 },
    { 1, stv, 5 },
    { 1, osm, 4 },
    { 1, stv, 3 },
    { 1, osm, 2 },
    { 1, stv, 1 },
    { 1, osm, 0 }
};
void loop() {
  playDualMelody(dualMelody1, sizeof(dualMelody1) / sizeof(dualMelody1[0]));
  playMelody(melody1, sizeof(melody1) / sizeof(melody1[0]));
}