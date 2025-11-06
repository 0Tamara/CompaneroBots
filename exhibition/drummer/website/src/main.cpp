#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <esp_log.h>
#include <ESP32Servo.h>
#include <FastLED.h>

//-pins-
#define LEFT_SERVO_PIN 12
#define RIGHT_SERVO_PIN 13
#define KICK_SERVO_PIN 32
#define LED_PIN_LEFT 27
#define LED_PIN_RIGHT 14
#define LED_PIN_KICK 33
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

#define MAX_SONGS 10  //max number of songs you can load


//-drivers-
Servo left_servo;  //80-0 = down-front
Servo right_servo;  //0-80 = down-front
Servo kick_servo;   //85-90-85 = kick
CRGB left_LEDs[SIDES_LED_COUNT];
CRGB right_LEDs[SIDES_LED_COUNT];
CRGB kick_LEDs[KICK_LED_COUNT];
CRGB eyes_LEDs[EYES_LED_COUNT];

//---WiFi---
const char* ssid     = "SPSE_drummer_ESP32";
const char* password = "gondek2025";
WebServer server(80);
IPAddress local_IP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

bool busy_playing = 0;

unsigned long timer_general;
unsigned long timer_bar;

//--csv reading--
size_t header_lines[MAX_SONGS] = {0};  //the first line of each song
size_t csv_pointer = 0;
File database;

//--leds--
uint color_eyes = 0x800080;
uint color_palette[6] = {0x800000,  //colors cycling over
                        0x404000,
                        0x008000,
                        0x004040,
                        0x000080,
                        0x400040};
int rising_color[3] = {0, 0, 0};  //color used for rising effects (during music)
bool blink_drums[3] = {0, 0, 0};  //rising effects active
int LEDs_pos[3] = {0, 0, SIDES_LED_COUNT-1};  //position on the LED ring
bool rising[3] = {1, 1, 1};  //rising / lowering
bool miss_out[3] = {0, 0, 0};  //missing out every other step to go slower
TaskHandle_t Task1;

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

//---change colors when the drum gets hit---
void changeColors(int drum/*0-left; 1-kick; 2-right*/)
{
  if(rising_color[drum] < 5)
    rising_color[drum]++;
  else
    rising_color[drum] = 0;
  rising[drum] = 1;
  blink_drums[drum] = 1;
}

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
    {
      left_servo.write(LEFT_SERVO_HIT);
      changeColors(0);
    }
    if ((current_bar.kick_notes & 1<<i) > 0)
    {
      kick_servo.write(KICK_SERVO_HIT);
      changeColors(1);
    }
    if ((current_bar.right_notes & 1<<i) > 0)
    {
      right_servo.write(RIGHT_SERVO_HIT);
      changeColors(2);
    }

    delay(100);
    //-release keys-
    left_servo.write(LEFT_SERVO_RELEASE);
    kick_servo.write(KICK_SERVO_RELEASE);
    right_servo.write(RIGHT_SERVO_RELEASE);
    while (millis() - timer_bar <= current_song.note_length * (current_song.notes_per_bar - i));
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
    if(header_lines[i] < 2)
      break;

    loadSongHeader(i);
    http_message += current_song.name + ",";
  }
  server.send(200, "text/plain", http_message);
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

//---loop for LED effects (during music)---
void loop_2(void* parameter)
{
  //---loop2---
  while(true)
  {
    if(blink_drums[0])  //left effect active
    {
      if(rising[0])  //lighting up
      {
        left_LEDs[LEDs_pos[0]] = color_palette[rising_color[0]];
        LEDs_pos[0] ++;

        if(LEDs_pos[0] == SIDES_LED_COUNT)  //prepare values for turn off stage
        {
          rising[0] = 0;
          LEDs_pos[0] = SIDES_LED_COUNT-1;
        }
      }
      else  //turning off
      {
        left_LEDs[LEDs_pos[0]] = 0;
        LEDs_pos[0] --;

        if(LEDs_pos[0] < 0)  //revert values
        {
          rising[0] = 1;
          blink_drums[0] = 0;
          LEDs_pos[0] = 0;
          miss_out[0] = 0;
        }
      }
    }

    if(blink_drums[1])  //kick effect active
    {
      if(rising[1])  //lighting up
      {
        kick_LEDs[LEDs_pos[1]] = color_palette[rising_color[1]];
        kick_LEDs[LEDs_pos[1]+1] = color_palette[rising_color[1]];
        kick_LEDs[(KICK_LED_COUNT-1)-LEDs_pos[1]] = color_palette[rising_color[1]];  //going from both sides to middle
        kick_LEDs[(KICK_LED_COUNT-1)-LEDs_pos[1]-1] = color_palette[rising_color[1]];
        LEDs_pos[1] +=2;

        if(LEDs_pos[1] > KICK_LED_COUNT/2-1)  //prepare values for turn off stage
        {
          rising[1] = 0;
          LEDs_pos[1] = KICK_LED_COUNT/2-1;
        }
      }
      else  //turning off
      {
        kick_LEDs[LEDs_pos[1]] = 0;
        kick_LEDs[(KICK_LED_COUNT-1)-LEDs_pos[1]] = 0;
        LEDs_pos[1] --;

        if(LEDs_pos[1] < 0)  //revert values
        {
          rising[1] = 1;
          blink_drums[1] = 0;
          LEDs_pos[1] = 0;
          miss_out[1] = 0;
        }
      }
    }

    if(blink_drums[2])  //right effect active
    {
      if(rising[2])
      {
        right_LEDs[LEDs_pos[2]] = color_palette[rising_color[2]];
        LEDs_pos[2] --;

        if(LEDs_pos[2] < 0)  //prepare values for turn off stage
        {
          rising[2] = 0;
          LEDs_pos[2] = 0;
        }
      }
      else
      {
        right_LEDs[LEDs_pos[2]] = 0;
        LEDs_pos[2] ++;

        if(LEDs_pos[2] == SIDES_LED_COUNT)  //revert values
        {
          rising[2] = 1;
          blink_drums[2] = 0;
          LEDs_pos[2] = SIDES_LED_COUNT-1;
          miss_out[2] = 0;
        }
      }
    }
    
    FastLED.show();
    delay(10);
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

  //-create loop 2-
  xTaskCreatePinnedToCore(
      loop_2, /* Function to implement the task */
      "Task1", /* Name of the task */
      10000,  /* Stack size in words */
      NULL,  /* Task input parameter */
      0,  /* Priority of the task */
      &Task1,  /* Task handle. */
      0); /* Core where the task should run */

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

  //-init LEDs and blink white-
  FastLED.addLeds<WS2811, LED_PIN_LEFT, GRB>(left_LEDs, SIDES_LED_COUNT);
  FastLED.addLeds<WS2811, LED_PIN_KICK, GRB>(kick_LEDs, KICK_LED_COUNT);
  FastLED.addLeds<WS2811, LED_PIN_RIGHT, GRB>(right_LEDs, SIDES_LED_COUNT);
  FastLED.addLeds<WS2811, LED_PIN_EYES, GRB>(eyes_LEDs, EYES_LED_COUNT);

  for (int i = 0; i < 54; i++) {
    if (i < SIDES_LED_COUNT) right_LEDs[i] = 0x808080;
    if (i < SIDES_LED_COUNT) left_LEDs[i] = 0x808080;
    if (i < KICK_LED_COUNT) kick_LEDs[i] = 0x808080;
  }
  FastLED.show();

  delay(100);
  for (int i = 0; i < 54; i++) {
    if (i < SIDES_LED_COUNT) right_LEDs[i] = 0x000000;
    if (i < SIDES_LED_COUNT) left_LEDs[i] = 0x000000;
    if (i < KICK_LED_COUNT) kick_LEDs[i] = 0x000000;
  }
  FastLED.show();
}

void loop()
{
  server.handleClient();
}