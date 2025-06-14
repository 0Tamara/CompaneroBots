#include <Adafruit_NeoPixel.h>

#define PIN_LAVY  27
#define PIN_PRAVY 14
#define PIN_KOPAK 32
#define PIN_EYES  23

#define PRAVY_BUBON 36
#define LAVY_BUBON  36
#define KOPAK       54
#define EYES        50

Adafruit_NeoPixel pravy(PRAVY_BUBON, PIN_PRAVY, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel lavy(LAVY_BUBON, PIN_LAVY, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel hlavny(KOPAK, PIN_KOPAK, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel eyes_LEDs(EYES, PIN_EYES, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  pravy.begin();
  delay(20);
  lavy.begin();
  delay(20);
  hlavny.begin();
  delay(20);
  eyes_LEDs.begin();
  delay(20);

  for (int i = 0; i < 54; i++) {
    if (i < PRAVY_BUBON) pravy.setPixelColor(i, 32, 32, 32);
    delay(20);
    if (i < LAVY_BUBON) lavy.setPixelColor(i, 32, 32, 32);
    delay(20);
    if (i < KOPAK) hlavny.setPixelColor(i, 32, 32, 32);
    delay(20);
    if (i < EYES) eyes_LEDs.setPixelColor(i, 32, 32, 32);
    delay(20);
    pravy.show();
    delay(20);
    lavy.show();
    delay(20);
    hlavny.show();
    delay(20);
    eyes_LEDs.show();
    delay(20);
  }
}

void loop() {
  ledky_vedlajsie();
  hlavny_bubon();
}

// Zapína LED na všetkých pásikoch naraz
void ledky_vedlajsie() {
  int max_led = max(PRAVY_BUBON, LAVY_BUBON);  // Najväčší počet LED

  for (int i = 0; i < max_led; i++) {
    if (i < PRAVY_BUBON) pravy.setPixelColor(i, 128, 0, 0);
    if (i < LAVY_BUBON) lavy.setPixelColor(i, 128, 0, 0);

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
    //hlavny.setPixelColor(i, 128, 0, 0);
    //hlavny.show();
    delay(50);
  }
  for (int i = KOPAK - 1; i >= 0; i--) {
    //hlavny.setPixelColor(i, 0, 0, 0);
    //hlavny.show();
    delay(50);
  }
}
