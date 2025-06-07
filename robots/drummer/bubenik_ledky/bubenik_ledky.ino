#include <Adafruit_NeoPixel.h>

#define PIN_LAVY 27
#define PIN_PRAVY 14
#define PIN_KOPAK 32

#define PRAVY_BUBON 36
#define LAVY_BUBON 36
#define KOPAK 54

Adafruit_NeoPixel pravy(PRAVY_BUBON, PIN_PRAVY, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel lavy(LAVY_BUBON, PIN_LAVY, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel hlavny(KOPAK, PIN_KOPAK, NEO_GRB + NEO_KHZ800);

void setup() {
  pravy.begin();
  lavy.begin();
  hlavny.begin();

  for (int i = 0; i < 54; i++) {
    if (i < PRAVY_BUBON) pravy.setPixelColor(i, 255, 255, 255);
    if (i < LAVY_BUBON) lavy.setPixelColor(i, 255, 255, 255);
    if (i < KOPAK) hlavny.setPixelColor(i, 255, 255, 255);
  }

  pravy.show();
  lavy.show();
  hlavny.show();
  delay(5000);
}

void loop() {
  ledky_vedlajsie();
  hlavny_bubon();
}

// Zapína LED na všetkých pásikoch naraz
void ledky_vedlajsie() {
  int max_led = max(PRAVY_BUBON, LAVY_BUBON);  // Najväčší počet LED

  for (int i = 0; i < max_led; i++) {
    if (i < PRAVY_BUBON) pravy.setPixelColor(i, 255, 0, 0);
    if (i < LAVY_BUBON) lavy.setPixelColor(i, 255, 0, 0);

    pravy.show();
    lavy.show();
    delay(50);
  }

  for (int i = max_led - 1; i >= 0; i--) {
    if (i < PRAVY_BUBON) pravy.setPixelColor(i, 0, 0, 0);
    if (i < LAVY_BUBON) lavy.setPixelColor(i, 0, 0, 0);

    pravy.show();
    lavy.show();
    delay(50);
  }
}
void hlavny_bubon() {
  for (int i = 0; i < KOPAK; i++) {
    hlavny.setPixelColor(i, 255, 0, 0);
    hlavny.show();
    delay(50);
  }
  for (int i = KOPAK - 1; i >= 0; i--) {
    hlavny.setPixelColor(i, 0, 0, 0);
    hlavny.show();
    delay(50);
  }
}
