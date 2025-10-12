#include <Arduino.h>
#include <FastAccelStepper.h>
#include <Adafruit_PWMServoDriver.h>
#include <Wire.h>
#include <esp_log.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <SPIFFS.h>

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

//-timing-
int tempo = 105;  //bpm
int note_lenght = 15000/tempo;  //4/4: ((60,000/bpm)*4)/16
unsigned long bar_timer = millis();
unsigned long timer;  //!!!TEMP!!!

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

//---functions---
void moveToPos()  //move hands into position from structure
{
  leftHand.stepper->moveTo(stepsPerNote * leftHand.pos[0] + stepsPerOctave * leftHand.pos[1]);
  rightHand.stepper->moveTo(stepsPerNote * rightHand.pos[0] + stepsPerOctave * rightHand.pos[1]);
}
void moveHome()  //move to the sides
{
  leftHand.stepper->moveTo(0);
  rightHand.stepper->moveTo(stepsPerOctave * 4);
  while(leftHand.stepper->isRunning() || rightHand.stepper->isRunning());
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
  if(note == C && (octave == 4 || octave == 7))  //last note on hands
  {
    pos_steps -= stepsPerOctave;
    servo_addr = 8;
  }
  else if(octave == 4)  //right hand is shifted by 1 to the right
  {
    
    pos_steps += stepsPerNote;
    servo_addr = 16-note;
  }
  else
    servo_addr = 15-note;

  if(note + octave*7 <= 28)  //if note is on the left from C4, play with left hand
  {
    //-move hand into position-
    leftHand.stepper->moveTo(pos_steps);
    while(leftHand.stepper->isRunning());
    //-play 1 note-
    pca9685left.setPWM(servo_addr, 0, leftHand.pressValue);
    delay(75);
    pca9685left.setPWM(servo_addr, 0, leftHand.releaseValue);
    Serial.printf("Left hand pos %d; servo %d\n", pos_steps/stepsPerNote, servo_addr);
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
    Serial.printf("Right hand pos %d; servo %d\n", pos_steps/stepsPerNote, servo_addr);
  }
}

void setup()
{
  //--debugging--
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  //--init I2C--
  esp_log_level_set("i2c.master", ESP_LOG_NONE);
  if (!Wire.begin(21, 22))
    Serial.println("!! SPIFFS init error");
  else
    Serial.println("** SPIFFS started");

  //--init SPIFFS--
  if (!SPIFFS.begin(true))
    Serial.println("!! SPIFFS init error");
  else
    Serial.println("** SPIFFS started");

  //--init pca9685 (hand servos driver)--
  pca9685right.begin();
  pca9685left.begin();
  pca9685right.setPWMFreq(50);
  pca9685left.setPWMFreq(50); 
  for (int i = 8; i <= numServos; i++){
    pca9685right.setPWM(i, 0, rightHand.releaseValue);
    pca9685left.setPWM(i, 0, leftHand.releaseValue); // 0 stupnov
  }
  //--init steppers--
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
  leftHand.stepper->setCurrentPosition(stepsPerOctave * 2);  //starting on the left end (C2)
  
  rightHand.stepper->setDirectionPin(rightHandDirPin);
  rightHand.stepper->setEnablePin(rightHandEnPin);
  rightHand.stepper->setAutoEnable(true);

  rightHand.stepper->setSpeedInHz(speedInHz);
  rightHand.stepper->setAcceleration(acceleration);
  rightHand.stepper->setCurrentPosition(stepsPerOctave * 6);  //starting on the right end (C6)
}

void loop()
{
  timer = millis();
  File database = SPIFFS.open("/songs.csv", "r");
  //if (!database) return;

  int lines = 0;
  String line;

  database.seek(3988);
  while (database.available()) {
    line = database.readStringUntil('\n');
    //Serial.print("Line:  ");
    //Serial.println(line);
    lines ++;
  }
  database.close();
  Serial.printf(">>%d lines read in %dms\n", lines, millis()-timer);
  delay(1000);
}