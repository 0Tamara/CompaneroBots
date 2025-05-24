#include <ESP32Servo.h>

#define numServos 8
#define rezerva 50 //treba odskusat

Servo servos[numServos];
int noteCount = 0;  
unsigned long lastTime;
int bluetooth_signal = 0; // 
//casy
int time = 1000;
int osm = time / 8;
int stv = time / 4;
int pol = time / 2;
int cel = time;
//serva
int rightServoPins[numServos] = { 2, 3, 4, 5, 6, 7, 8, 9 };  // dal som nech viem menit podla schemy
//krokovy motor
int stepPin = 10;
int dirPin = 11;
int stepsPerOctave = 250;  //toto treba vypocitat alebo odskuksat - spravim sam samozrejme

int currentOctave = 0;

void setup() {
  for (int i = 0; i < numServos; i++) {
    servos[i].attach(rightServoPins[i]);
  }

  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
}

//posun medzi oktavami, jednoducho pokial nota nie je v oktave, tak sa posunie na taku oktavu, na ktorej nota je
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
void playNote(int noteIndex1, int noteIndex2, int noteIndex3, int octave, int wait) {
  moveToOctave(octave);
  int notes[3] = { noteIndex1, noteIndex2, noteIndex3 };
  lastTime = millis();
  while (millis() - lastTime <= wait - (rezerva * noteCount)) {
    noteCount = 0;
    for (int i = 0; i < 3; i++) {
      if (notes[i] != -1 && notes[i] < numServos) {
        servos[notes[i]].write(90);
        noteCount += 1;
      }
    }
  }
  lastTime = millis();
  while (millis() - lastTime <= (rezerva * noteCount)) {
    for (int i = 0; i < 3; i++) {
      if (notes[i] != -1 && notes[i] < numServos) {
        servos[notes[i]].write(0);
      }
    }
  }
}


//tu bude cela melodia, zatial tu je ze prazdnô
void playMelody(int melody[][5], int length) {

  for (int i = 0; i < length; i++) {
    int octave = melody[i][0];
    int wait = melody[i][1];
    int note1 = melody[i][2];
    int note2 = melody[i][3];
    int note3 = melody[i][4];
    playNote(note1, note2, note3, octave, wait);
  }
}

int melody1[][5] = {
  //hore
  { 1, osm, 0, -1, -1 },
  { 1, osm, 1, -1, -1 },
  { 1, osm, 2, -1, -1 },
  { 1, osm, 3, -1, -1 },
  { 1, osm, 4, -1, -1 },
  { 1, osm, 5, -1, -1 },
  { 1, osm, 6, -1, -1 },
  { 1, osm, 7, -1, -1 },
  //dolu
  { 1, osm, 7, -1, -1 },
  { 1, osm, 6, -1, -1 },
  { 1, osm, 5, -1, -1 },
  { 1, osm, 4, -1, -1 },
  { 1, osm, 3, -1, -1 },
  { 1, osm, 2, -1, -1 },
  { 1, osm, 1, -1, -1 },
  { 1, osm, 0, -1, -1 }
};
int melody2[][5] = {
//hore
  { 2, cel, 0, -1, -1 },
  { 2, cel, 2, -1, -1 },
  { 2, cel, 4, -1, -1 },
  { 2, cel, 2, -1, -1 },
  { 2, cel, 4, -1, -1 },
  { 2, cel, 7, -1, -1 },
  { 3, cel, 0, -1, -1 },
  { 3, cel, 2, -1, -1 },
  { 3, cel, 4, -1, -1 },
//dolu
  { 3, cel, 4, -1, -1 },
  { 3, cel, 2, -1, -1 },
  { 3, cel, 0, -1, -1 },
  { 2, cel, 7, -1, -1 },
  { 2, cel, 4, -1, -1 },
  { 2, cel, 2, -1, -1 },
  { 2, cel, 4, -1, -1 },
  { 2, cel, 2, -1, -1 },
  { 2, cel, 0, -1, -1 }
};
  
void loop() {
  if (bluetooth_signal == 1) {
    playMelody(melody1, sizeof(melody1) / sizeof(melody1[0]));
  }
  if (bluetooth_signal == 2)  
  {
    playMelody(melody2, sizeof(melody2) / sizeof(melody2[0]));
  }
}
