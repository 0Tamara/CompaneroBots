#include <FastLED.h>

#define PIN_LAVY  27
#define PIN_PRAVY 14
#define PIN_KOPAK 32
#define PIN_EYES  23

#define PRAVY_BUBON 36
#define LAVY_BUBON  36
#define KOPAK       54
#define EYES        50

CRGB pravy[PRAVY_BUBON];
CRGB lavy[LAVY_BUBON];
CRGB kick[KOPAK];
CRGB eyes[EYES];

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<WS2811, PIN_PRAVY, GRB>(pravy, PRAVY_BUBON);
  FastLED.addLeds<WS2811, PIN_LAVY, GRB>(lavy, LAVY_BUBON);
  FastLED.addLeds<WS2811, PIN_KOPAK, GRB>(kick, KOPAK);
  FastLED.addLeds<WS2811, PIN_EYES, GRB>(eyes, EYES);
}

void loop() {
  ledky_vedlajsie();
  hlavny_bubon();
}

// Zapína LED na všetkých pásikoch naraz
void ledky_vedlajsie() {
  int max_led = max(PRAVY_BUBON, LAVY_BUBON);  // Najväčší počet LED

  for (int i = 0; i < max_led; i++) {
    if (i < PRAVY_BUBON) pravy[i] = 0x0000FF;
    if (i < LAVY_BUBON) lavy[i] = 0x00FF00;

    FastLED.show();
    delay(20);
  }

  for (int i = max_led - 1; i >= 0; i--) {
    if (i < PRAVY_BUBON) pravy[i] = 0x000000;
    if (i < LAVY_BUBON) lavy[i] = 0x000000;

    FastLED.show();
    delay(20);
  }
}
void hlavny_bubon() {
  for (int i = 0; i < KOPAK; i++) {
    kick[i] = 0xFF0000;
    FastLED.show();
    delay(20);
  }
  for (int i = KOPAK - 1; i >= 0; i--) {
    kick[i] = 0x000000;
    FastLED.show();
    delay(20);
  }
}
