#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <Wire.h>
#include <esp_log.h>
#include <Adafruit_PWMServoDriver.h>
#include <FastAccelStepper.h>

//---pins---
#define leftHandStepPin   5 
#define leftHandDirPin    16 
#define leftHandEnPin     18
#define rightHandStepPin  4
#define rightHandDirPin   17
#define rightHandEnPin    15
//---speed/position values---
#define numServos 16
#define SERVOMIN  125
#define SERVOMAX  575
#define stepsPerNote 929
#define stepsPerOctave 6498
#define speedInHz 15000
#define acceleration 40000

#define MAX_SONGS 10  //max number of songs you can load

enum moveNotes {C=0, D=1, E=2, F=3, G=4, A=5, H=6};  //number of notes to the right from C

//---WiFi---
const char* ssid     = "SPSE_ESP32_main";
const char* password = "gondek2025";
WebServer server(80);
IPAddress local_IP(192, 168, 0, 1);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

bool busy_playing = 0;

//---timers---
unsigned long timer_general;
unsigned long timer_bar;

//---servo drivers---
Adafruit_PWMServoDriver pca9685right(0x41, Wire);
Adafruit_PWMServoDriver pca9685left(0x40, Wire);

FastAccelStepperEngine stepper_driver = FastAccelStepperEngine();  //stepper driver

//---hands structures---
struct Hand
{
  FastAccelStepper* stepper;
  int pressValue[8];
  int releaseValue[8];
};
Hand leftHand = {
  .stepper = NULL,
  .pressValue = {
    SERVOMAX - 80,
    SERVOMAX - 50,
    SERVOMAX - 60,
    SERVOMAX - 60,
    SERVOMAX - 50,
    SERVOMAX - 70,
    SERVOMAX - 50,
    SERVOMAX - 60},
  .releaseValue = {
    SERVOMAX - 30,
    SERVOMAX - 0,
    SERVOMAX - 10,
    SERVOMAX - 10,
    SERVOMAX - 0,
    SERVOMAX - 20,
    SERVOMAX - 0,
    SERVOMAX - 10}
};
Hand rightHand = {
  .stepper = NULL,
  .pressValue = {
    SERVOMIN + 50,
    SERVOMIN + 70,
    SERVOMIN + 50,
    SERVOMIN + 70,
    SERVOMIN + 50,
    SERVOMIN + 50,
    SERVOMIN + 60,
    SERVOMIN + 50},
  .releaseValue = {
    SERVOMIN + 0,
    SERVOMIN + 20,
    SERVOMIN + 0,
    SERVOMIN + 20,
    SERVOMIN + 0,
    SERVOMIN + 0,
    SERVOMIN + 10,
    SERVOMIN + 0}
};

//---csv reading---
size_t song_lines[MAX_SONGS] = {0};  //the first line of each song
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

int decodeNote(char note_coded)  //decode letter to int  C->0; H->6
{  //if not a note letter ->8
  switch(note_coded)
  {
    case 'C': return C; break;
    case 'D': return D; break;
    case 'E': return E; break;
    case 'F': return F; break;
    case 'G': return H; break;
    case 'A': return A; break;
    case 'H': return H; break;
    default: return 8; break;
  }
}

//---load from FFS functions---
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
      song_lines[song_index] = (database.position());  //save where the empty line is
      song_index ++;
      if(song_index >= MAX_SONGS)
        break;
    }
  }
  database.close();
}
void loadSongHeader(int which)  //read the first line of a song and save into current_song
{
  if(song_lines[which] == 0)
    return;
  String csv_element;
  int line_data_index = 0;  //which element on the line
  int tempo;

  database = SPIFFS.open("/songs.csv", "r");
  database.seek(song_lines[which]);
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
void loadBar()  //read 1 line from pointer
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
      csv_element_decoded = decodeNote(csv_element[0]);
      if(csv_element_decoded == 8)
        csv_element_decoded = csv_element.toInt();

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

//---hands movement---
void moveToPos()  //move hands into position from current_bar
{
  leftHand.stepper->moveTo(stepsPerNote * current_bar.pos_left[0] + stepsPerOctave * current_bar.pos_left[1]);
  rightHand.stepper->moveTo(stepsPerNote * current_bar.pos_right[0] + stepsPerOctave * current_bar.pos_right[1]);
}
void moveHome()  //move to the sides
{
  leftHand.stepper->moveTo(stepsPerOctave * 2);
  rightHand.stepper->moveTo(stepsPerOctave * 6);
  while(leftHand.stepper->isRunning() || rightHand.stepper->isRunning()) server.handleClient();
}

//---playing functions---
void playBar()  //play 1 bar of a song  from current_bar
{
  timer_bar = millis();
  moveToPos();
  for (int i = 0; i < current_song.notes_per_bar; i++)
  {    
    for (int j = 0; j < 8; j++)
    {
      if ((current_bar.fingers_left[i] & 1<<j) && !(leftHand.stepper->isRunning()))
        pca9685left.setPWM(j+8, 0, leftHand.pressValue[j]);
      if ((current_bar.fingers_right[i] & 1<<j) && !(rightHand.stepper->isRunning()))
        pca9685right.setPWM(j+8, 0, rightHand.pressValue[j]);
    }
    
    delay(75);
    for (int j=0; j<8; j++)  //release keys
    {
      pca9685right.setPWM(j+8, 0, rightHand.releaseValue[j]);
      pca9685left.setPWM(j+8, 0, leftHand.releaseValue[j]);
    }
    while (millis() - timer_bar <= current_song.note_length * i) server.handleClient();
  }
}
void playNote(byte note, byte octave)  //play 1 note
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
    while(leftHand.stepper->isRunning()) server.handleClient();
    //-play 1 note-
    pca9685left.setPWM(servo_addr, 0, leftHand.pressValue[servo_addr-8]);
    delay(75);
    pca9685left.setPWM(servo_addr, 0, leftHand.releaseValue[servo_addr-8]);
    Serial.printf("Left hand pos %d; servo %d\n", pos_steps/stepsPerNote, servo_addr);
  }
  else
  {
    //-move hand into position-
    rightHand.stepper->moveTo(pos_steps);
    while(rightHand.stepper->isRunning()) server.handleClient();
    //-play 1 note-
    pca9685right.setPWM(servo_addr, 0, rightHand.pressValue[servo_addr-8]);
    delay(75);
    pca9685right.setPWM(servo_addr, 0, rightHand.releaseValue[servo_addr-8]);
    Serial.printf("Right hand pos %d; servo %d\n", pos_steps/stepsPerNote, servo_addr);
  }
}
void playSong(int song_index)
{
  loadSongHeader(song_index);
  loadBar();
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
}

//---Web server functions---
void loadWebSite()  //load html website for client
{
  File html_file = SPIFFS.open("/index.html", "r");
  if(!html_file)
  {
    server.send(404, "text/plain", "File not found");
    Serial.println("!! html not found");
    return;
  }
  server.streamFile(html_file, "text/html");
  html_file.close();
  Serial.println("** html file loaded");
}
//--http requests--
void loadSongsInfo()
{
  String http_message;
  for(int i=0; i<MAX_SONGS; i++)
  {
    if(song_lines[i] < 2)
      break;

    loadSongHeader(i);
    http_message += i + "," + current_song.name + ",";
  }
}
void readHttpKey()
{
  int note = decodeNote(server.arg("note")[0]);
  int octave = server.arg("octave").toInt();
  playNote(note, octave);
  moveHome();
  server.send(200, "text/plain", "done");
  Serial.printf("Played note %d %d", note, octave);
}
void readHttpSong()
{
  int song_number = server.arg("song").toInt()-1;
  server.send(202, "text/plain", "processing");
  busy_playing = 1;
  playSong(song_number);
  busy_playing = 0;
  Serial.printf("Played song %d", song_number);
}
void updateStatus()
{
  if(busy_playing)
    server.send(202, "text/plain", "processing");
  else
    server.send(200, "text/plain", "done");
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
    pca9685right.setPWM(i, 0, rightHand.releaseValue[i-8]);
    pca9685left.setPWM(i, 0, leftHand.releaseValue[i-8]); // 0 stupnov
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

  //--init WiFi--
  WiFi.softAPConfig(local_IP, gateway, subnet);
  if(!WiFi.softAP(ssid, password))
    Serial.println("!! WiFi init error");
  else
  {
    Serial.println("** WiFi started");
    Serial.print(">> IP address: ");
    Serial.println(local_IP);
  }
  //--handling http requests--
  server.on("/", loadWebSite);  //load html website on "/" request (loading website)
  server.on("/songs-info", loadSongsInfo);  //load on web site startup
  server.on("/key-press", readHttpKey);  //read from website piano
  server.on("/play-song", readHttpSong);  //read from website song selection
  server.on("/status", updateStatus);  //check for busy status
  server.serveStatic("/", SPIFFS, "/");  //load any other not identified files (csss, js)
  server.onNotFound([]()  //if no request matches
  {
    if (server.uri().endsWith("/generate_204") ||  //silence checking for Internet connection
        server.uri().endsWith("/gen_204") ||
        server.uri().endsWith("/hotspot-detect.html") ||
        server.uri().endsWith("/ncsi.txt") ||
        server.uri().endsWith("/success.txt") ||
        server.uri().endsWith("/connecttest.txt"))
    {
      server.send(204, "text/plain", "");  //"connected, no Internet"
      return;
    }
    server.send(404, "text/plain", "Not found");
    Serial.print("!! Couldn't load ");
    Serial.print(server.uri());
    Serial.println(server.args());
  });

  server.begin();  //init web server
  Serial.println("** server running");
}

void loop()
{
  server.handleClient();
}