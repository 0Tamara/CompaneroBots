#include <esp_log.h>
#include <FastAccelStepper.h>
#include <Adafruit_PWMServoDriver.h>
#include <Wire.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>

//-speed/position values-
#define numServos 16
#define SERVOMIN  125
#define SERVOMAX  575
#define stepsPerNote 929
#define stepsPerOctave 6498
#define speedInHz 15000
#define acceleration 40000
//-pins-
#define leftHandStepPin   5 
#define leftHandDirPin    16 
#define leftHandEnPin     18
#define rightHandStepPin  4
#define rightHandDirPin   17
#define rightHandEnPin    15

enum moveNotes {C=0, D=1, E=2, F=3, G=4, A=5, H=6};  //number of notes to the right from C

//-esp-now mess structures-
typedef struct recieve_struct {
  byte song;
  byte time;
} recieve_struct;
recieve_struct recieved_data;

//-timing-
int tempo = 105;  //bpm
int note_lenght = 15000/tempo;  //4/4: ((60,000/bpm)*4)/16
unsigned long bar_timer = millis();

//-servo drivers-
Adafruit_PWMServoDriver pca9685right(0x41, Wire);
Adafruit_PWMServoDriver pca9685left(0x40, Wire);

FastAccelStepperEngine stepper_driver = FastAccelStepperEngine();  //stepper driver

//-hands structures-
struct Hand
{
  FastAccelStepper* stepper;
  byte bar[16];  //16 sets of 8 notes
  byte pos[2];  //position on the keyboard [note, octave]
  int pressValue;  //value to set servos
  int releaseValue;  //value to set servos
};
Hand leftHand = {
  .stepper = NULL,
  .bar = {0},
  .pos = {C, 0},
  .pressValue = SERVOMAX - 100,
  .releaseValue = SERVOMAX
};
Hand rightHand = {
  .stepper = NULL,
  .bar = {0},
  .pos = {C, 4},
  .pressValue = SERVOMIN + 100,
  .releaseValue = SERVOMIN
};

/*int havasiFreedomRightPosition1[]{A, 2};
int havasiFreedomLeftPosition1[]{A, 0};
int havasiFreedomRight1[]{
  0b00000000, //cel pomlcka
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
};

int havasiFreedomLeft1[] = {
  0b10000000, //osm
  0b00000000,
  0b00000000, // sest pomlcka
  0b00000001, //osm
  0b00000000, 
  0b00000000, //sest pomlcka
  0b10000001, //osm
  0b00000000,
  0b10000001, //stv
  0b00000000,
  0b00000000,
  0b00000000, 
  0b10000001, //stv
  0b00000000,
  0b00000000,
  0b00000000,
};
int havasiFreedomRight5[] = {
  0b10101000, // osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b10101000, // osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b10100000, // stv
  0b00000000,
  0b00000000,
  0b00000000,
  0b10000000, //sest
  0b01000000, //sest
  0b00100000, //sest
  0b01010000, //osm
  0b00000000, 
  0b00001000, //sest
}; 
int havasiFreedomLeft5[] = {
  0b10000001, //osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b10000001, //osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b10000001, //osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b10000001, //osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b10000001, //stv
  0b00000000,
  0b00000000, 
  0b00000000,
};
int havasiFreedomRight6[] = {
  0b10100000, //stv
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000, // osm pomlcka
  0b00000000,
  0b00000000, //stvrtova pomlcka
  0b00000000,
  0b00000000,
  0b00000000,
  0b01000000, //sest
  0b00100000, //sest
  0b01010000, //osm
  0b00000000,
  0b00101000, //sest
  0b00000000,
};
int havasiFreedomRight7[] = {
  //este len zacatok a uz si uvedomujem jak krasne vonka svieti slnko
  0b10101000, // osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b10100000, // osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b00000000, // stv pomlcka
  0b00000000,
  0b00000000,
  0b00000000,
  0b10000000, //sest
  0b01000000, //sest
  0b10100000, //osm
  0b00000000,
  0b00000000, // sest pomlcka
  0b01010000, // sest
  
};
int havasiFreedomRight8[] = {
  0b10100000, //stv
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000, //osm pomlcka
  0b00000000,
  0b00000000, //stv pomlcka
  0b00000000,
  0b00000000,
  0b00000000,
  0b10000000, //sest
  0b01000000, //sest
  0b10100000, //osm
  0b00000000,
  0b10010000, //osm
  0b00000000,
};

int havasiFreedomLeftPosition9[] = {F, 0};
int havasiFreedomRight9[] = {
  //uz len kuuus... 
  0b10101000, // osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b10101000, // osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b00000000, // stv
  0b00000000,
  0b00000000,
  0b00000000,
  0b10000000, //sest
  0b01000000, //sest
  0b10100000, //osm
  0b00000000,
  0b00000000, //sest pomlcka 
  0b01010000, //sest
};
int havasiFreedomRight10[] = {
  0b10100000, //stv
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000, //osm pomlcka
  0b00000000,
  0b00000000, //stv pomlcka
  0b00000000,
  0b00000000,
  0b00000000,
  0b10000000, //sest
  0b01000000, //sest
  0b10100000, //osm
  0b00000000,
  0b01010000, //osm
  0b00000000,
};
int havasiFreedomRight11[] = {
  0b10101000, // osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b10100000, // osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b00000000, // stv pomlcka
  0b00000000,
  0b00000000,
  0b00000000,
  0b10000000, //sest
  0b01000000, //sest
  0b10100000, //osm
  0b00000000,
  0b01010000, // osm
  0b00000000, 
};
int havasiFreedomRightPosition12[] = {G, 2};
int havasiFreedomLeftPosition12[] = {E, 0};
int havasiFreedomRight12[] = {
  //jak to bolo? Tech tisic mil tech tisic mil ma jeden daco, a jeden cil
  0b00100010, //osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b00100010, //osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b00100010, //osm
  0b00000000,
  0b01000010, //stv
  0b00000000, 
  0b00000000, 
  0b00000000, 
  0b01000010, //stv
  0b00000000, 
  0b00000000, 
  0b00000000, 
};
int havasiFreedomLeft12[] = {
  0b10000001, //osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b10000001, //osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b10000001, //osm
  0b00000000,
  0b10000001, //stv
  0b00000000, 
  0b00000000, 
  0b00000000, 
  0b10000001, //stv
  0b00000000, 
  0b00000000, 
  0b00000000,   
};
int fireballRightPosition1[] = {G, 2};
int fireballRight1[] = {
  0b10000000, //stv
  0b00000000, 
  0b00000000, 
  0b00000000,  
  0b00000001, //osm
  0b00000000, 
  0b00000001, //osm
  0b00000000, 
  0b00000010, //sest
  0b00000010, //osm
  0b00000000, 
  0b00000010, //sest
  0b00000000, //stv pomlcka
  0b00000000, 
  0b00000000, 
  0b00000000, 
};
int fireballLeftPosition1[] = {G, 0};
int fireballLeft1[] = {
  0b10000000, //stv
  0b00000000, 
  0b00000000, 
  0b00000000,  
  0b00001000, //osm
  0b00000000,  
  0b00000010, //osm
  0b00000000,  
  0b00000001, //stv
  0b00000000, 
  0b00000000, 
  0b00000000,  
  0b00000010, //osm
  0b00000000,  
  0b00001000, //osm
  0b00000000,  
};
int fireballRight2[] = {
  0b00000000, //osm pomlcka
  0b00000000, 
  0b10000000, //osm nota
  0b00000000,  
  0b00000001, //osm
  0b00000000, 
  0b00000001, //osm
  0b00000000, 
  0b00000010, //sest
  0b00000010, //osm
  0b00000000, 
  0b00000010, //sest
  0b00000000, //stv pomlcka
  0b00000000, 
  0b00000000, 
  0b00000000, 
};
int fireballRightPosition5[]{D, 4};
int fireballRight5[] = {
  0b00010000, //stv
  0b00000000, 
  0b00000000, 
  0b00000000,  
  0b00001000, //osm
  0b00000000, 
  0b00100000, //osm
  0b00000000, 
  0b00010000, //sest
  0b00000000, //osm
  0b00000000, 
  0b10000000, //sest
  0b00000000, //stv pomlcka
  0b00000000, 
  0b00000000, 
  0b00000000, 
};
int fireballRight6[] = {
  0b00000000, //osm pomlcka
  0b00000000, 
  0b00010000, //osm 
  0b00000000,  
  0b00001000, //osm
  0b00000000, 
  0b00100000, //osm
  0b00000000, 
  0b00010000, //sest
  0b00000000, //osm
  0b00000000, 
  0b10000000, //sest
  0b00000000, //stv pomlcka
  0b00000000, 
  0b00000000, 
  0b00000000, 
};
int fireballRight8[] = {
  0b00000000, //osm pomlcka
  0b00000000, 
  0b00010000, //osm 
  0b00000000,  
  0b00001000, //osm
  0b00000000, 
  0b00100000, //osm
  0b00000000, 
  0b00010000, //sest
  0b00000000, //osm
  0b00000000, 
  0b00000001, //sest
  0b00000000, //stv pomlcka
  0b00000000, 
  0b00000000, 
  0b00000000, 
};
int stupnicaLeftPosition[]{C, 0};
int stupnicaRightPosition[]{C, 2};
int stupnicaRightAndLeft[]{
  0b10000000,
  0b01000000,
  0b00100000,
  0b00010000,
  0b00001000,
  0b00000100,
  0b00000010,
  0b00000001,
  0b00000001,
  0b00000010,
  0b00000100,
  0b00001000,
  0b00010000,
  0b00100000,
  0b01000000,
  0b10000000,
};*/
int osudovaLeftPosition[]{C, 0};
int osudovaRightPosition[]{C, 2};
int osudovaRightAndLeft1[]{
  0b00000000,
  0b00000000,
  0b00000000, //osm pomlcka
  0b00000000,
  0b00010000,
  0b00000000,
  0b00000000, //osm 
  0b00000000,
  0b00010000,
  0b00000000,
  0b00000000, //osm
  0b00000000,
  0b00010000,
  0b00000000,
  0b00000000, // osm
  0b00000000,
};
int osudovaRightAndLeft2[]{
  0b01000000, // osm
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
};
int osudovaRightAndLeft3[]{
  
  0b00000000, //osm pomlcka
  0b00000000,
  0b00000000,
  0b00000000,
  0b00100000, //osm 
  0b00000000,
  0b00000000,
  0b00000000,
  0b00100000, 
  0b00000000,
  0b00000000,//osm
  0b00000000,
  0b00100000, // osm
  0b00000000,
  0b00000000,
  0b00000000,
};
int osudovaRightAndLeft4[]{
  0b10000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000, // osm
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
};

void moveToPos()  //move hands into position
{
  rightHand.stepper->moveTo(stepsPerNote * rightHand.pos[0] + stepsPerOctave * rightHand.pos[1]);
  leftHand.stepper->moveTo(stepsPerNote * leftHand.pos[0] + stepsPerOctave * leftHand.pos[1]);
}

void playBar()  //play 1 bar of a song
{
  moveToPos();
  //-play 16 notes (1 bar)-
  for (int i = 0; i < 16; i++)
  {    
    for (int j = 0; j < 8; j++)
    {
      if ((rightHand.bar[i] & 1<<j) && !(rightHand.stepper->isRunning()))
        pca9685right.setPWM(j+8, 0, rightHand.pressValue);
      if ((leftHand.bar[i] & 1<<j) && !(leftHand.stepper->isRunning()))
        pca9685left.setPWM(j+8, 0, leftHand.pressValue);
    }
    
    delay(75);
    //-release keys-
    for (int j=8; j<16; j++)
    {
      pca9685right.setPWM(j, 0, rightHand.releaseValue);
      pca9685left.setPWM(j, 0, leftHand.releaseValue);
    }
    while (millis() - bar_timer <= note_lenght * i);
  }
}

void playNote(byte note, byte octave)
{
  int pos_steps = stepsPerOctave*octave;
  byte servo_addr;
  if(octave < 2)
    servo_addr = 15-note;
  else if(note == C && octave == 2)  //last left hand note
  {
    pos_steps = stepsPerOctave;
    servo_addr = 8;
  }
  else if(note == C && octave == 5)  //last right hand note
  {
    pos_steps = stepsPerOctave*4;
    servo_addr = 9;
  }
  else  //right hand is shifted by 1 to the right
  {
    pos_steps += stepsPerNote;
    servo_addr = 16-note;
  }
  if(note + octave*7 <= 14)  //if note is on the left from C2, play with left hand
  {
    //-move hand into position-
    leftHand.stepper->moveTo(pos_steps);
    while(leftHand.stepper->isRunning());
    //-play 1 note-
    pca9685left.setPWM(servo_addr, 0, leftHand.pressValue);
    delay(75);
    pca9685left.setPWM(servo_addr, 0, leftHand.releaseValue);
  }
  else
  {
    //-move hand into position-
    rightHand.stepper->moveTo(pos_steps);
    while(rightHand.stepper->isRunning());
    //-play 1 note-
    pca9685right.setPWM(servo_addr, 0, rightHand.pressValue);
    delay(75);
    pca9685right.setPWM(servo_addr, 0, rightHand.releaseValue);
  }
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {/*
  bar_timer = millis();
  memcpy(&recieved_data, incomingData, sizeof(recieved_data));
  Serial.printf("song = %d\n", recieved_data.song);
  Serial.printf("time = %d\n", recieved_data.time);
  if(recieved_data.song == 1)
  {
    rightHand.stepper->moveTo(0);
    while (rightHand.stepper->isRunning()) {
    } 
    leftHand.stepper->moveTo(0);
    while (leftHand .stepper->isRunning()) {
    }
  }
  if(recieved_data.song == 2)
  {
    tempo = 270; 
    note_lenght = 15000/tempo; 
    for(int i=0; i<16; i++)
    {
      leftHand.bar[i] = osudovaRightAndLeft1[i];
      rightHand.bar[i] = osudovaRightAndLeft1[i];
    }
    for(int i=0; i<2; i++)
    {
      leftHand.pos[i] = osudovaLeftPosition[i];
      rightHand.pos[i] = osudovaRightPosition[i];
    }
    playBar();
    while(millis() - bar_timer <= note_lenght*16 ){}
    //prvy takt
    bar_timer = millis();
    for(int i=0; i<16; i++)
    {
      leftHand.bar[i] = osudovaRightAndLeft2[i];
      rightHand.bar[i] =osudovaRightAndLeft2[i];
    }
    playBar();
    while(millis() - bar_timer <= note_lenght*16 ){}
    //druhy takt
    bar_timer = millis();
    for(int i=0; i<16; i++)
    {
      leftHand.bar[i] = osudovaRightAndLeft3[i];
      rightHand.bar[i] =osudovaRightAndLeft3[i];
    }
    playBar();
    while(millis() - bar_timer <= note_lenght*16 ){}
    //treti takt
    bar_timer = millis();
    for(int i=0; i<16; i++)
    {
      leftHand.bar[i] = osudovaRightAndLeft4[i];
      rightHand.bar[i] =osudovaRightAndLeft4[i];
    }
    playBar();
    while(millis() - bar_timer <= note_lenght*16 ){}
    //stvrty takt
    data_to_send.end = 1;
    esp_now_send(camAddr, (uint8_t *) &data_to_send, sizeof(data_to_send));
    Serial.printf("Data sended: %d\n",data_to_send.end );
  }
  if(recieved_data.song == 3)
  {
    bar_timer = millis();
    tempo = 80;
    note_lenght = 15000/tempo;
    for(int i=0; i<16; i++)
    {
      leftHand.bar[i] = stupnicaRightAndLeft[i];
      rightHand.bar[i] = stupnicaRightAndLeft[i];
    }
    for(int i=0; i<2; i++)
    {
      leftHand.pos[i] = stupnicaLeftPosition[i];
      rightHand.pos[i] = stupnicaRightPosition[i];
    }
    playBar();
    data_to_send.end = 1;
    esp_now_send(camAddr, (uint8_t *) &data_to_send, sizeof(data_to_send));
    Serial.printf("Data sended: %d\n",data_to_send.end );
  }
  if(recieved_data.song == 4)
  {
    tempo = 105;
    note_lenght = tempo/16;
    if(recieved_data.time == 1)
    {
      for(int i=0; i<16; i++)
      {
        leftHand.bar[i] = havasiFreedomLeft1[i];
        rightHand.bar[i] = havasiFreedomRight1[i];
      }
      for(int i=0; i<2; i++)
      {
        leftHand.pos[i] = havasiFreedomLeftPosition1[i];
        rightHand.pos[i] = havasiFreedomRightPosition1[i];
      }
    }
    if(recieved_data.time == 5)
    {
      for(int i=0; i<16; i++)
      {
        leftHand.bar[i] = havasiFreedomLeft5[i];
        rightHand.bar[i] = havasiFreedomRight5[i];
      }
    }
    //piaty takt
    if(recieved_data.time == 6)
    {
      for(int i=0; i<16; i++)
      {
        leftHand.bar[i] = havasiFreedomLeft5[i];
        rightHand.bar[i] = havasiFreedomRight6[i];
      }
    }
    // siesty takt
    if(recieved_data.time == 7)
    {
      for(int i=0; i<16; i++)
      {
        leftHand.bar[i] = havasiFreedomLeft5[i];
        rightHand.bar[i] = havasiFreedomRight7[i];
      }
    }
    //siedmy takt
    if(recieved_data.time == 8)
    {
      for(int i=0; i<16; i++)
      {
        leftHand.bar[i] = havasiFreedomLeft5[i];
        rightHand.bar[i] = havasiFreedomRight8[i];
      }
    }
    //osmy takt
    if(recieved_data.time == 9)
    {
      for(int i=0; i<16; i++)
      {
        leftHand.bar[i] = havasiFreedomLeft5[i];
        rightHand.bar[i] = havasiFreedomRight9[i];
      }
      for(int i=0; i<2; i++)
      {
        leftHand.pos[i] = havasiFreedomLeftPosition9[i];
      }
    }
    //deviaty takt
    if(recieved_data.time ==10)
    {
      for(int i=0; i<16; i++)
      {
        leftHand.bar[i] = havasiFreedomLeft5[i];
        rightHand.bar[i] = havasiFreedomRight10[i];
      }
    }
    //desiaty takt
    if(recieved_data.time ==11)
    {
      for(int i=0; i<16; i++)
      {
        leftHand.bar[i] = havasiFreedomLeft5[i];
        rightHand.bar[i] = havasiFreedomRight11[i];
      }
    }
    //jedenasty takt
    if(recieved_data.time == 12)
    {
      for(int i=0; i<16; i++)
      {
        leftHand.bar[i] = havasiFreedomLeft12[i];
        rightHand.bar[i] = havasiFreedomRight12[i];
      }
      for(int i=0; i<2; i++)
      {
        leftHand.pos[i] = havasiFreedomLeftPosition12[i];
        rightHand.pos[i] = havasiFreedomRightPosition12[i];
      }
    }
    //dvanasty a posledny takt
    playBar();
  }
  if(recieved_data.song == 5)
  {
    if(recieved_data.time == 1)
    {
      tempo = 115; 
      note_lenght = 15000/tempo; 
      for(int i=0; i<16; i++)
      {
        leftHand.bar[i] = fireballLeft1[i];
        rightHand.bar[i] = fireballRight1[i];
      }
      for(int i=0; i<2; i++)
      {
        leftHand.pos[i] = fireballLeftPosition1[i];
        rightHand.pos[i] = fireballRightPosition1[i];
      }
    }
    if(recieved_data.time == 2)
    {
      for(int i=0; i<16; i++)
      {
        rightHand.bar[i] = fireballRight2[i];
      }
    }
    if(recieved_data.time == 3)
    {
      for(int i=0; i<16; i++)
      {
        rightHand.bar[i] = fireballRight1[i];
      }
    }
    if(recieved_data.time == 4)
    {
      
      for(int i=0; i<16; i++)
      {
        rightHand.bar[i] = fireballRight2[i];
      } 
    }
    if(recieved_data.time == 5)
    {
      for(int i=0; i<16; i++)
      {
        rightHand.bar[i] = fireballRight5[i];
      }
      for(int i=0; i<2; i++)
      {
        rightHand.pos[i] = fireballRightPosition5[i];
      }
    }
    if(recieved_data.time == 6)
    {
      for(int i=0; i<16; i++)
      {
        rightHand.bar[i] = fireballRight6[i];
      }
    }
    if(recieved_data.time == 7)
    {
      for(int i=0; i<16; i++)
      {
        rightHand.bar[i] = fireballRight5[i];
      }
    }
    if(recieved_data.time == 8)
    {
      for(int i=0; i<16; i++)
      {
        rightHand.bar[i] = fireballRight8[i];
      }
    }
    if(recieved_data.time == 9)
    {
      for(int i=0; i<16; i++)
      {
        rightHand.bar[i] = fireballRight5[i];
      }
    }
    if(recieved_data.time == 10)
    {
      for(int i=0; i<16; i++)
      {
        rightHand.bar[i] = fireballRight6[i];
      }
    }
    if(recieved_data.time == 11)
    {
      for(int i=0; i<16; i++) 
      {
        rightHand.bar[i] = fireballRight5[i];
      }
    }
    if(recieved_data.time == 12)
    {
      bar_timer = millis();
      rightHand.stepper->moveTo(stepsPerOctave * 2);
      while (rightHand.stepper->isRunning()) 
      {
      } 
      leftHand.stepper->moveTo(0);
      while (leftHand.stepper->isRunning()) 
      {
      }
      while(millis() - bar_timer <= note_lenght*16){}
    }
    playBar();
  }*/
}

void setup()
{
  //-init serial-
  Serial.begin(115200);
  esp_log_level_set("i2c.master", ESP_LOG_NONE);
  Wire.begin(21, 22);
  //-init pca9685 (hand servos driver)-
  pca9685right.begin();
  pca9685left.begin();
  pca9685right.setPWMFreq(50);
  pca9685left.setPWMFreq(50); 
  for (int i = 8; i <= numServos; i++){
    pca9685right.setPWM(i, 0, rightHand.releaseValue);
    pca9685left.setPWM(i, 0, leftHand.releaseValue); // 0 stupnov
  }
  //-init steppers-
  stepper_driver.init();
  leftHand.stepper = stepper_driver.stepperConnectToPin(leftHandStepPin);
  rightHand.stepper = stepper_driver.stepperConnectToPin(rightHandStepPin);
  if (rightHand.stepper == NULL || leftHand.stepper == NULL)
    Serial.println("Chyba pri pripojeni krokovych motorov.");

  leftHand.stepper->setDirectionPin(leftHandDirPin);
  leftHand.stepper->setEnablePin(leftHandEnPin);
  leftHand.stepper->setAutoEnable(true);

  leftHand.stepper->setSpeedInHz(speedInHz);
  leftHand.stepper->setAcceleration(acceleration);
  leftHand.stepper->setCurrentPosition(0);  //starting on the left end (C0)
  
  rightHand.stepper->setDirectionPin(rightHandDirPin);
  rightHand.stepper->setEnablePin(rightHandEnPin);
  rightHand.stepper->setAutoEnable(true);

  rightHand.stepper->setSpeedInHz(speedInHz);
  rightHand.stepper->setAcceleration(acceleration);
  rightHand.stepper->setCurrentPosition(stepsPerOctave * 4);  //starting on the right end (C4)

  //-init WiFi & read MAC address-
  WiFi.mode(WIFI_STA);
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  Serial.printf("My MAC address: {0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X}\n",
                baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
  //-init ESP-NOW-
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  //-register recv callback-
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}

void loop()
{
  /*bar_timer = millis();
  tempo = 270; 
  note_lenght = 15000/tempo; 
  for(int i=0; i<16; i++)
  {
    leftHand.bar[i] = osudovaRightAndLeft1[i];
    rightHand.bar[i] = osudovaRightAndLeft1[i];
  }
  for(int i=0; i<2; i++)
  {
    leftHand.pos[i] = osudovaLeftPosition[i];
    rightHand.pos[i] = osudovaRightPosition[i];
  }
  moveToPos();
  bar_timer = millis();
  playBar();
  while(millis() - bar_timer <= note_lenght*16 ){}
  //prvy takt
  bar_timer = millis();
  for(int i=0; i<16; i++)
  {
    leftHand.bar[i] = osudovaRightAndLeft2[i];
    rightHand.bar[i] =osudovaRightAndLeft2[i];
  }
  playBar();
  while(millis() - bar_timer <= note_lenght*16 ){}
  //druhy takt
  bar_timer = millis();
  for(int i=0; i<16; i++)
  {
    leftHand.bar[i] = osudovaRightAndLeft3[i];
    rightHand.bar[i] =osudovaRightAndLeft3[i];
  }
  playBar();
  while(millis() - bar_timer <= note_lenght*16 ){}
  //treti takt
  bar_timer = millis();
  for(int i=0; i<16; i++)
  {
    leftHand.bar[i] = osudovaRightAndLeft4[i];
    rightHand.bar[i] =osudovaRightAndLeft4[i];
  }
  playBar();
  while(millis() - bar_timer <= note_lenght*16 ){}*/

  for(int i=0; i<5; i++)
  {
    playNote(C, i);
    delay(100);
    playNote(D, i);
    delay(100);
    playNote(E, i);
    delay(100);
    playNote(F, i);
    delay(100);
    playNote(G, i);
    delay(100);
    playNote(A, i);
    delay(100);
    playNote(H, i);
    delay(100);
  }
}