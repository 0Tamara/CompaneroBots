#include <Servo.h>

#define numServos 8
Servo servos[numServos];


//noty
int time = 1000;
int osm = time / 8;
int stv = time / 4;
int pol = time/2;
int cel = time;

int servoPins[numServos] = {2, 3, 4, 5, 6, 7, 8, 9}; // dal som nech viem menit podla schemy

int stepPin = 10;
int dirPin = 11;
int stepsPerOctave = 200;  //toto treba vypocitat alebo odskuksat

int currentOctave = 0;  

void setup() {
  for (int i = 0; i < numServos; i++) {
    servos[i].attach(servoPins[i]);
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
//toto len stlaci notu
void playNote(int noteIndex1, int noteIndex2, int noteIndex3, int octave, int wait) {
  moveToOctave(octave);
  int notes[3] = {noteIndex1, noteIndex2, noteIndex3};
  // simuluj stlačenie noty
  for(int i = 0; i < 3; i++){
    if(notes[i] != -1) {
      int note = notes[i];
      servos[note].write(30);
      delay(wait / 6);
      servos[note].write(90);
      delay(wait / 6);
    }
    else{
      delay(wait / 3);
    }
  }
}


//tu bude cela melodia, zatial tu je ze prazdnô
void playMelody() {
  int melody[][5] = {
    {5, -1, -1, 2, osm}, 
    {0, 1, 2, 1, stv}, 
    {6, 4, 2, 1, pol}, 
    {6, 1, 5, 1, pol}  
  };

  int length = sizeof(melody) / sizeof(melody[0]);

  for (int i = 0; i < length; i++) {
    int note1 = melody[i][0];
    int note2 = melody[i][1];
    int note3 = melody[i][2];
    int octave = melody[i][3];
    int wait = melody[i][4];
    playNote(note1, note2, note3, octave, wait);
  }
}

void loop() {
  playMelody();
}
