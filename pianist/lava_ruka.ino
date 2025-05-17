#include <ESP32Servo.h>

#define numServos 8
#define rezerva 100
#define stepsPerNote 25;
#define stepsPerOctave 200
unsigned long lastTime;

//na pamatanie aktualnej oktavy
int currentOctave = 0;
int currentNote = 0;
//serva
Servo servos[numServos];
int servoPins[numServos] = { 2, 3, 4, 5, 6, 7, 8, 9 };  
//krok motor
int stepPin = 10;
int dirPin = 11;

unsigned long lastTime;
// casy
int time = 1000;
int osm = time / 8;
int stv = time / 4;
int pol = time / 2;
int cel = time;

void setup() {
  for (int i = 0; i < numServos; i++) {
    servos[i].attach(servoPins[i]);
  }
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
}



void moveToNote(int targetNote, int targetOctave) {
  int direction = (targetOctave * stepsPerOctave + targetNote * stepsPerNote) - (curretOctave * stepsPerOctave + currentNote * stepsPerNote);

  if (direction == 0) return;

  digitalWrite(dirPin, direction > 0 ? HIGH : LOW);
  int steps = abs(direction);

  for (int i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(800);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(800);
  }

  currentOctave = octave;
}

//tu mozem spravit aj to, ze bude rovnaky kod ako na pravu ruku, ale vlastne mal by som noteIndex1 - 8, a posuval by som sa priamo na notu
void playDualNote(int noteIndex1 int octave, int wait) {
  moveToNote(noteIndex1, octave);
  lastTime = millis();
  while (millis() - lastTime <= (wait - rezerva)) {
    servos[noteIndex1].write(90);
    servos[noteIndex2].write(90);
  }
  lastTime = millis();
  while (millis() - lastTime <= rezerva) {
    servos[noteIndex1].write(0);
    servos[noteIndex2].write(0);
  }
}
void playMelody() {
  int melody[][3] = {
    { 1, 1, stv },
    { 5, 0, osm },
    { 4, 1, pol },
    { 1, 2, pol },
    { 7, 0, osm } 
  };

  int length = sizeof(melody) / sizeof(melody[0]);

  for (int i = 0; i < length; i++) {
    int note = melody[i][0];
    int octave = melody[i][1];
    int wait = melody[i][2];
    playDualNote(note, octave, wait);
  }
}

void loop() {
  playMelody();
}
