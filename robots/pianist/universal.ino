#include <ESP32Servo.h>

#define numServos 8
#define rezerva 100
#define stepsPerNote 25
#define stepsPerOctave 200
unsigned long lastTime;

//na pamatanie aktualnej oktavy
int currentOctave = 0;
int currentNote = 0;
int noteCount = 0;
int speedDelay = 0;
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

void moveToNote(int targetNote, int targetOctave) {
  int direction = (targetOctave * stepsPerOctave + targetNote * stepsPerNote) - 
                    (currentOctave * stepsPerOctave + currentNote * stepsPerNote);

  if (direction == 0) return;

  digitalWrite(dirPin, direction > 0 ? HIGH : LOW);
  int steps = abs(direction);

  for (int i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(50);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(50);
  }

  currentOctave = targetOctave;
  currentNote = targetNote;
}
void playNote(int wait, int note1, int note2, int note3) {
  int notes[3] = {note1, note2, note3};
  lastTime = millis();
  noteCount = 0;#include <ESP32Servo.h>

#define numServos 8
#define rezerva 100
#define stepsPerNote 25
#define stepsPerOctave 200
unsigned long lastTime;

//na pamatanie aktualnej oktavy
int currentOctave = 0;
int currentNote = 0;
int noteCount = 0;
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

void moveToNote(int targetNote, int targetOctave) {
  int direction = (targetOctave * stepsPerOctave + targetNote * stepsPerNote) - 
                    (currentOctave * stepsPerOctave + currentNote * stepsPerNote);

  if (direction == 0) return;

  digitalWrite(dirPin, direction > 0 ? HIGH : LOW);
  int steps = abs(direction);
  speedDelay = map(steps, 0, 800, 200, 30);
  speedDelay = constrain(speedDelay, 30, 500);

  for (int i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(speedDelay);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(speedDelay);
  }

  currentOctave = targetOctave;
  currentNote = targetNote;
}
void playNote(int wait, int note1, int note2, int note3) {
  int notes[3] = {note1, note2, note3};
  lastTime = millis();
  noteCount = 0;
  for(int i = 0; i < 3; i++){
    if(notes[i] != -1 && notes[i] < numServos) {
        noteCount +=1;
    }
  }
    while (millis() - lastTime <= wait - rezerva * noteCount) {
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
void playMelody(int melody[][4], int length) {
  for (int i = 0; i < length; i++) {
    int wait = melody[i][0];
    int note1 = melody[i][1];
    int note2 = melody[i][2];
    int note3 = melody[i][3];
    playNote(wait, note1, note2, note3);
  }
}
int melody1[][4] = {
    {osm, -1, -1, 2},
    {stv, 2, -1, 5}

};
void loop() {
    moveToNote(5, 1);
    playMelody(melody1, sizeof(melody1) / sizeof(melody1[0]));
}
  for(int i = 0; i < 3; i++){
    if(notes[i] != -1 && notes[i] < numServos) {
        noteCount +=1;
    }
  }
    while (millis() - lastTime <= wait - rezerva * noteCount) {
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
void playMelody(int melody[][4], int length) {
  for (int i = 0; i < length; i++) {
    int wait = melody[i][0];
    int note1 = melody[i][1];
    int note2 = melody[i][2];
    int note3 = melody[i][3];
    playNote(wait, note1, note2, note3);
  }
}
int melody1[][4] = {
    {osm, -1, -1, 2},
    {stv, 2, -1, 5}

};
void loop() {
    moveToNote(5, 1);
    playMelody(melody1, sizeof(melody1) / sizeof(melody1[0]));
}