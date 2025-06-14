#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
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
//Adafruit_NeoPixel eyesx(EYES, PIN_EYES, NEO_GRB + NEO_KHZ800);
//Adafruit_NeoPixel eyesx(KOPAK, PIN_KOPAK, NEO_GRB + NEO_KHZ800);
Adafruit_NeoMatrix eyesx = Adafruit_NeoMatrix(10, 5, PIN_EYES,
  NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
  NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
  NEO_GRB            + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  pravy.begin();
  lavy.begin();
  hlavny.begin();
  eyesx.begin();

  //pravy.show();
  //lavy.show();
  //hlavny.show();
  //eyes_LEDs.show();
}

void loop() {
  for (int i = 0; i < 300; i++)
  {
    pravy.setPixelColor(i, 32, 32, 32);
    eyesx.fillScreen(0x0080FF);
    pravy.show();
    delay(20);
    //Serial.println("1");
    pravy.show();
    //Serial.println("2");
    //lavy.show();
    //Serial.println("3");
    //hlavny.show();
    //Serial.println("4");
    eyesx.show();
  }
}
/*
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
}*/
