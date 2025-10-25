#include <Arduino.h>
#include <ESP32Servo.h>
#include <FastLED.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <SPIFFS.h>

//-pins-
#define LEFT_SERVO_PIN 12
#define RIGHT_SERVO_PIN 13
#define KICK_SERVO_PIN 32
#define LED_PIN_LEFT 27
#define LED_PIN_RIGHT 14
#define LED_PIN_KKICK 33
#define LED_PIN_EYES 23
//-values-
#define LEFT_SERVO_RELEASE 20
#define LEFT_SERVO_HIT 30
#define RIGHT_SERVO_RELEASE 60
#define RIGHT_SERVO_HIT 50
#define KICK_SERVO_RELEASE 85
#define KICK_SERVO_HIT 90
#define SIDES_LED_COUNT 36  //count of leds on both side drums
#define KICK_LED_COUNT 54
#define EYES_LED_COUNT 50

#define MAX_SONGS 10

//-drivers-
Servo left_servo;  //80-0 = down-front
Servo right_servo;  //0-80 = down-front
Servo kick_servo;   //85-90-85 = kick
CRGB left_LEDs[SIDES_LED_COUNT];
CRGB right_LEDs[SIDES_LED_COUNT];
CRGB kick_LEDs[KICK_LED_COUNT];
CRGB eyes_LEDs[EYES_LED_COUNT];

unsigned long timer_general;
unsigned long timer_bar;

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
  uint16_t left_notes;
  uint16_t kick_notes;
  uint16_t right_notes;
};
bar_type current_bar;

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
  uint16_t csv_element_decoded;

  database = SPIFFS.open("/songs.csv", "r");
  database.seek(csv_pointer);
  while(database.available())  //read the line
  {
    csv_element = database.readStringUntil(',');

    if(line_data_index == 0)
      current_bar.end = csv_element.toInt();
    else
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
      switch(line_data_index)
      {
        case 1:
          current_bar.left_notes = csv_element_decoded; break;
        case 2:
          current_bar.kick_notes = csv_element_decoded; break;
        case 3:
          current_bar.right_notes = csv_element_decoded; break;
      }
    }

    if(database.peek() == 13)  //if the next char is '\n'
      break;
    line_data_index++;
  }
  database.read();  //cross the new line
  csv_pointer = database.position();
  database.close();
}

void playBar()
{
  timer_bar = millis();

  for (int i = current_song.notes_per_bar-1; i >= 0; i--)
  {
    if ((current_bar.left_notes & 1<<i) > 0)
      left_servo.write(LEFT_SERVO_HIT);
    if ((current_bar.kick_notes & 1<<i) > 0)
      kick_servo.write(KICK_SERVO_HIT);
    if ((current_bar.right_notes & 1<<i) > 0)
      right_servo.write(RIGHT_SERVO_HIT);
    
    delay(100);
    //-release keys-
    left_servo.write(LEFT_SERVO_RELEASE);
    kick_servo.write(KICK_SERVO_RELEASE);
    right_servo.write(RIGHT_SERVO_RELEASE);
    while (millis() - timer_bar <= current_song.note_length * (current_song.notes_per_bar - i));
  }
}

void setup()
{
  //--debugging--
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);

  //--init SPIFFS--
  if (!SPIFFS.begin(true))
    Serial.println("!! SPIFFS init error");
  else
    Serial.println("** SPIFFS started");
  loadDatabase();
  //-init servos-
  right_servo.attach(RIGHT_SERVO_PIN);
  left_servo.attach(LEFT_SERVO_PIN);
  kick_servo.attach(KICK_SERVO_PIN);
  
  kick_servo.write(KICK_SERVO_RELEASE);
  right_servo.write(RIGHT_SERVO_RELEASE);
  left_servo.write(LEFT_SERVO_RELEASE);
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
  delay(10000);
}