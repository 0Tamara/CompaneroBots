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
void playNote(int noteIndex, int octave, int wait) {
  moveToOctave(octave);
  
  // simuluj stlačenie noty
  servos[noteIndex].write(30);  // dolu
  delay(wait / 2);                
  servos[noteIndex].write(90);  // hore
  delay(wait / 2);
}


//tu bude cela melodia, zatial tu je ze prazdnô
void playMelody() {
  int melody[][3] = {
    {5, 0, osm}, // A
    {0, 1, stv}, // C 
    {6, 3, pol}, // H
    {6, 1, pol}  // H
  };

  int length = sizeof(melody) / sizeof(melody[0]);

  for (int i = 0; i < length; i++) {
    int note = melody[i][0];
    int octave = melody[i][1];
    int wait = melody[i][2];
    playNote(note, octave, wait);
  }
}

void loop() {
  playMelody();
}
