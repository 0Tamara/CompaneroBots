#include <Adafruit_NeoPixel.h>

#define LED_PIN 25        // Pin pripojený na hlavný LED pásik
#define LED_COUNT 21      // Počet LED diód
#define LED_SENSOR_PIN 27 // Pin pre druhý LED pásik (senzor)
#define LED_SENSOR_COUNT 1

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel sensor_strip(LED_SENSOR_COUNT, LED_SENSOR_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  strip.begin();
  strip.clear();  // Vypne všetky LEDky
  strip.show();

  sensor_strip.begin();
  sensor_strip.clear();  // Vypne LED na senzore
  sensor_strip.setPixelColor(0, sensor_strip.Color(0, 0, 0)); // Nastavíme čiernu farbu (vypnutie)
  sensor_strip.show();
}

void loop() {
  knightRiderEffect();
}

// Funkcia pre efekt "Night Rider"
void knightRiderEffect() {
  int mid = LED_COUNT / 2;  // Nájdeme strednú LED
  
  // Prvá fáza: od stredu smerom von
  for (int step = 0; step <= mid; step++) {
    strip.clear();
    
    if (mid - step >= 0) strip.setPixelColor(mid - step, 255, 0, 0); // Červená
    if (mid + step < LED_COUNT) strip.setPixelColor(mid + step, 255, 0, 0);

    strip.show();  // Aktualizujeme LEDky až po nastavení všetkých
    delay(45);
  }

  // Druhá fáza: z okrajov späť do stredu
  for (int step = mid; step >= 0; step--) {
    strip.clear();
    
    if (mid - step >= 0) strip.setPixelColor(mid - step, 255, 0, 0);
    if (mid + step < LED_COUNT) strip.setPixelColor(mid + step, 255, 0, 0);

    strip.show();  // Aktualizujeme LEDky až po nastavení všetkých
    delay(45);
  }
}
