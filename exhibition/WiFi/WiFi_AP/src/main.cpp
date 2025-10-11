#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <WebServer.h>

//---WiFi values---
const char* ssid     = "SPSE_ESP32_main";
const char* password = "sikanaNaStudentov2025";
WebServer server(80);
IPAddress local_IP(192, 168, 0, 1);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

//---load html website---
void loadWebSite()
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

void readHttpButton()
{
  int pressed_button = server.arg("button").toInt();  //read the value of parameter "button"
  Serial.printf(">> button number %d was pressed\n", pressed_button);
  digitalWrite(2, HIGH);
  delay(50);
  digitalWrite(2, LOW);
  server.send(200, "text/plain", "recieved");
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
  server.on("/pressed", readHttpButton);
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
    Serial.println("!! request (file) missing");
  });

  server.begin();  //init web server
  Serial.println("** server running");
}

void loop()
{
  server.handleClient();
}