#include <Arduino.h>
#include <FastAccelStepper.h>
#include <Adafruit_PWMServoDriver.h>
#include <Wire.h>
#include <esp_log.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <SPIFFS.h>

//-pins-
#define leftHandStepPin   5 
#define leftHandDirPin    16 
#define leftHandEnPin     18
#define rightHandStepPin  4
#define rightHandDirPin   17
#define rightHandEnPin    15
//-speed/position values-
#define numServos 16
#define SERVOMIN  125
#define SERVOMAX  575
#define stepsPerNote 929
#define stepsPerOctave 6498
#define speedInHz 15000
#define acceleration 40000

#define MAX_SONGS 10  //max number of songs you can load

enum moveNotes {C=0, D=1, E=2, F=3, G=4, A=5, H=6};  //number of notes to the right from C

//-timers-
unsigned long timer_general;
unsigned long timer_bar;

//-servo drivers-
Adafruit_PWMServoDriver pca9685right(0x41, Wire);
Adafruit_PWMServoDriver pca9685left(0x40, Wire);

FastAccelStepperEngine stepper_driver = FastAccelStepperEngine();  //stepper driver

//-hands structures-
struct Hand
{
  FastAccelStepper* stepper;
  //byte bar[16];  //16 sets of 8 notes
  //byte pos[2];  //position on the keyboard [note, octave]
  int pressValue;  //value to set servos
  int releaseValue;  //value to set servos
};
Hand leftHand = {
  .stepper = NULL,
  .pressValue = SERVOMAX - 70,
  .releaseValue = SERVOMAX
};
Hand rightHand = {
  .stepper = NULL,
  .pressValue = SERVOMIN + 60,
  .releaseValue = SERVOMIN
};

//--csv reading--
size_t header_lines[MAX_SONGS] = {0};  //the first line of each song
size_t csv_pointer = 0;
File database;

struct song
{
  String name;
  int length;  //in bars
  int note_length;  //in ms
  int notes_per_bar;  //number of notes in a bar
};
song current_song;

struct bar_type
{
  int end;  // till when this bar loops
  byte pos_left[2];  //[note, octave]
  byte pos_right[2];  //[note, octave]
  byte fingers_left[16];  //byte(8 fingers) of every 16th note
  byte fingers_right[16];  //byte(8 fingers) of every 16th note
};
bar_type current_bar;

//---functions---
void loadDatabase()  //read the file and save song starting lines
{
  database = SPIFFS.open("/songs.csv", "r");
  if(!database)
  {
    Serial.println("!! failed to load database");
    return;
  }
  Serial.println("** database open");

  String line;
  int song_index = 0;
  while (database.available())  //read the whole document
  {
    line = database.readStringUntil('\n');
    if(line.length() < 2)
    {
      header_lines[song_index] = (database.position());  //save where the empty line is
      song_index ++;
      if(song_index >= MAX_SONGS)
        break;
    }
  }
  database.close();
}
void loadSongHeader(int which)
{
  if(header_lines[which] == 0)
    return;
  String csv_element;
  int line_data_index = 0;  //which element on the line
  int tempo;

  database = SPIFFS.open("/songs.csv", "r");
  database.seek(header_lines[which]);
  while (database.available())  //read the line
  {
    csv_element = database.readStringUntil(',');

    switch(line_data_index)  //decode song header data
    {
      case 0:
        current_song.name = csv_element;
        break;
      case 1:
        tempo = csv_element.toInt();
        break;
      case 2:
        current_song.length = csv_element.toInt();
        break;
      case 3:
        if(csv_element == "4/4")
        {
          current_song.notes_per_bar = 16;
          current_song.note_length = 15000/tempo;  //((60,000/bpm)*4)/16
        }
        else if(csv_element == "6/8")
        {
          current_song.notes_per_bar = 12;
          current_song.note_length = 30000/tempo;  //((60,000/bpm)*6)/12
        }
        break;
    }

    if(database.peek() == 13)  //if the next char is '\n'
      break;
    line_data_index++;
  }
  database.read();  //cross the new line
  csv_pointer = database.position();
  database.close();
}
void loadBar()  //read from pointer
{
  String csv_element;
  int line_data_index = 0;  //which element on the line
  byte csv_element_decoded = 0;

  database = SPIFFS.open("/songs.csv", "r");
  database.seek(csv_pointer);
  while (database.available())  //read the line
  {
    csv_element = database.readStringUntil(',');

    if(line_data_index == 0)  //starting index
      current_bar.end = csv_element.toInt();
    else if(1 <= line_data_index && line_data_index <= 4)  //hand positions
    {
      switch(csv_element[0])
      {
        case 'C': csv_element_decoded = C; break;
        case 'D': csv_element_decoded = D; break;
        case 'E': csv_element_decoded = E; break;
        case 'F': csv_element_decoded = F; break;
        case 'G': csv_element_decoded = H; break;
        case 'A': csv_element_decoded = A; break;
        case 'H': csv_element_decoded = H; break;
        default: csv_element_decoded = csv_element.toInt(); break;
      }
      if(line_data_index <= 2)
        current_bar.pos_left[line_data_index-1] = csv_element_decoded;
      else
        current_bar.pos_right[line_data_index-3] = csv_element_decoded;
    }
    else  //finger presses
    {
      csv_element_decoded = 0;
      for(int i=0; i<csv_element.length(); i++)  //decode HEX to byte
      {
        csv_element_decoded *= 16;
        if(csv_element[i] <= '9')
          csv_element_decoded += csv_element[i] - 0x30;
        else
          csv_element_decoded += csv_element[i] - 55;
      }

      if(line_data_index <= 20)
        current_bar.fingers_left[line_data_index-5] = csv_element_decoded;
      else
        current_bar.fingers_right[line_data_index-21] = csv_element_decoded;
    }

    if(database.peek() == 13)  //if the next char is '\n'
      break;
    line_data_index++;
  }
  database.read();  //cross the new line
  csv_pointer = database.position();
  database.close();
}

void moveToPos()  //move hands into position from structure
{
  leftHand.stepper->moveTo(stepsPerNote * current_bar.pos_left[0] + stepsPerOctave * current_bar.pos_left[1]);
  rightHand.stepper->moveTo(stepsPerNote * current_bar.pos_right[0] + stepsPerOctave * current_bar.pos_right[1]);
}
void moveHome()  //move to the sides
{
  leftHand.stepper->moveTo(stepsPerOctave * 2);
  rightHand.stepper->moveTo(stepsPerOctave * 6);
  while(leftHand.stepper->isRunning() || rightHand.stepper->isRunning());
}

void playBar()  //play 1 bar of a song
{
  timer_bar = millis();
  moveToPos();
  for (int i = 0; i < current_song.notes_per_bar; i++)
  {    
    for (int j = 0; j < 8; j++)
    {
      if ((current_bar.fingers_left[i] & 1<<j) && !(leftHand.stepper->isRunning()))
        pca9685left.setPWM(j+8, 0, leftHand.pressValue);
      if ((current_bar.fingers_right[i] & 1<<j) && !(rightHand.stepper->isRunning()))
        pca9685right.setPWM(j+8, 0, rightHand.pressValue);
    }
    
    delay(75);
    //-release keys-
    for (int j=0; j<8; j++)
    {
      pca9685right.setPWM(j+8, 0, rightHand.releaseValue);
      pca9685left.setPWM(j+8, 0, leftHand.releaseValue);
    }
    while (millis() - timer_bar <= current_song.note_length * i);
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
    Serial.println("!! I2C init error");
  else
    Serial.println("** I2C started");

  //--init SPIFFS--
  if (!SPIFFS.begin(true))
    Serial.println("!! SPIFFS init error");
  else
    Serial.println("** SPIFFS started");
  loadDatabase();
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
  loadSongHeader(0);
  current_bar.end = 0;
  timer_general = millis();
  for(int i=0; i<current_song.length; i++)
  {
    if(i == current_bar.end)
      loadBar();
    playBar();
    Serial.printf("Played bar ending at %d; t= %d bars\n", current_bar.end, i);
    while(millis() - timer_general < current_song.note_length * current_song.notes_per_bar);
    timer_general = millis();
  }
  moveHome();
  delay(1000);
}