#include <Adafruit_NeoPixel.h>

#define PIN_LAVY 4
#define PIN_PRAVY 16
#define PIN_KOPAK 17        // Pin pripojený na hlavný LED pásik
#define PRAVY_BUBON 36      // Počet LED diód
#define LAVY_BUBON 36 // Pin pre druhý LED pásik (senzor)
#define KOPAK 54

Adafruit_NeoPixel pravy(PRAVY_BUBON, PIN_PRAVY, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel lavy(LAVY_BUBON, PIN_LAVY, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel hlavny(KOPAK, PIN_KOPAK, NEO_GRB + NEO_KHZ800);

void setup() {
  pravy.begin();
  pravy.clear();  // Vypne všetky LEDky
  pravy.show();

  lavy.begin();
  lavy.clear();  // Vypne všetky LEDky
  lavy.show();

  hlavny.begin();
  hlavny.clear();  // Vypne všetky LEDky
  hlavny.show();
}

void loop() {
  pravy_bubon();
  lavy_bubon();
  hlavny_bubon();
}

// Funkcia pre efekt "Night Rider"
void pravy_bubon() {
  for(int i = 0; i < PRAVY_BUBON; i++){
    pravy.setPixelColor(i, 255, 0, 0); 
    pravy.show();
    delay(50);
  }
  for(int i = PRAVY_BUBON; i > 0; i--){
    pravy.setPixelColor(i, 0, 0, 0); 
    pravy.show();
    delay(50);
  }
}

void lavy_bubon() {
  for(int i = 0; i < LAVY_BUBON; i++){
    lavy.setPixelColor(i, 255, 0, 0); 
    lavy.show();
    delay(50);
  }
  for(int i = LAVY_BUBON; i > 0; i--){
    lavy.setPixelColor(i, 0, 0, 0); 
    lavy.show();
    delay(50);
  }
}

void hlavny_bubon() {
  for(int i = 0; i < PRAVY_BUBON; i++){
    hlavny.setPixelColor(i, 255, 0, 0); 
    hlavny.show();
    delay(50);
  }
  for(int i = PRAVY_BUBON; i > 0; i--){
    hlavny.setPixelColor(i, 0, 0, 0); 
    hlavny.show();
    delay(50);
  }
}