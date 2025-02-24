#include <ESP32Servo.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN_4 26        // Changed from 18 to 4
#define LED_COUNT_4 50      // Changed from 50 to 4
#define LED_PIN_1   25
#define LED_PIN_2   2
#define LED_PIN_3   16

#define LED_COUNT_1 36
#define LED_COUNT_2 54
//36
//54

Adafruit_NeoPixel strip_1(LED_COUNT_1, LED_PIN_1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip_2(LED_COUNT_2, LED_PIN_2, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip_3(LED_COUNT_1, LED_PIN_3, NEO_GRB + NEO_KHZ800);

Adafruit_NeoPixel strip_4(LED_COUNT_4, LED_PIN_4, NEO_GRB + NEO_KHZ800);
Servo r, l;

// Color settings (easily changeable)
uint8_t red = 40, green = 1, blue = 2;  // Modify these values to change the color

// Timing variables
unsigned long previousMillis = 0;
const int interval = 2000;  // Millis interval for blinking

void setup() {
  strip_1.begin();
  strip_1.show();
  strip_2.begin();
  strip_2.show();
  strip_3.begin();
  strip_3.show();
  Serial.begin(9600);
  r.attach(5);
  l.attach(17);
  strip_4.begin();  // Now using strip_4 instead of strip_1
  strip_4.show();   // Initialize all pixels to off
}

void loop() {
  unsigned long currentMillis = millis();

  // Light up all LEDs (for the initial state)
  fillStrip(red, green, blue);
  delay(2000); // Initial delay before blinking starts
  
  // Blink effects (non-blocking)
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    blink(red, green, blue);
    blink2(red, green, blue);
    drums();
  }
}

void fillStrip(uint8_t red, uint8_t green, uint8_t blue) {
  for (int i = 0; i < LED_COUNT_4; i++) {  // Changed LED_COUNT_1 to LED_COUNT_4
    strip_4.setPixelColor(i, red, green, blue);  // Changed strip_1 to strip_4
  }
  strip_4.show();  // Show the updated color
}

void blink(uint8_t red, uint8_t green, uint8_t blue) {
  // Blink LEDs in reverse order (off in sections)
  for (int i = 20; i < 25; i++) strip_4.setPixelColor(i, 0, 0, 0); // Left eye
  for (int i = 45; i < 50; i++) strip_4.setPixelColor(i, 0, 0, 0); // Right eye
  strip_4.show();

  delay(50);

  // Now let's go down the LED sections
  for (int i = 15; i < 20; i++) strip_4.setPixelColor(i, 0, 0, 0);
  for (int i = 40; i < 45; i++) strip_4.setPixelColor(i, 0, 0, 0);
  strip_4.show();

  delay(50);

  for (int i = 10; i < 15; i++) strip_4.setPixelColor(i, 0, 0, 0);
  for (int i = 35; i < 40; i++) strip_4.setPixelColor(i, 0, 0, 0);
  strip_4.show();

  delay(50);

  for (int i = 5; i < 10; i++) strip_4.setPixelColor(i, 0, 0, 0);
  for (int i = 30; i < 35; i++) strip_4.setPixelColor(i, 0, 0, 0);
  strip_4.show();

  delay(50);

  for (int i = 0; i < 5; i++) strip_4.setPixelColor(i, 0, 0, 0);
  for (int i = 25; i < 30; i++) strip_4.setPixelColor(i, 0, 0, 0);
  strip_4.show();

  delay(50);
}

void blink2(uint8_t red, uint8_t green, uint8_t blue) {
  // Blink LEDs in reverse order (turning LEDs back on)
  for (int i = 0; i < 5; i++) strip_4.setPixelColor(i, red, green, blue); // Left eye
  for (int i = 25; i < 30; i++) strip_4.setPixelColor(i, red, green, blue); // Right eye
  strip_4.show();

  delay(50);

  for (int i = 5; i < 10; i++) strip_4.setPixelColor(i, red, green, blue);
  for (int i = 30; i < 35; i++) strip_4.setPixelColor(i, red, green, blue);
  strip_4.show();

  delay(50);

  for (int i = 10; i < 15; i++) strip_4.setPixelColor(i, red, green, blue);
  for (int i = 35; i < 40; i++) strip_4.setPixelColor(i, red, green, blue);
  strip_4.show();

  delay(50);

  for (int i = 15; i < 20; i++) strip_4.setPixelColor(i, red, green, blue);
  for (int i = 40; i < 45; i++) strip_4.setPixelColor(i, red, green, blue);
  strip_4.show();

  delay(50);

  for (int i = 20; i < 25; i++) strip_4.setPixelColor(i, red, green, blue);
  for (int i = 45; i < 50; i++) strip_4.setPixelColor(i, red, green, blue);
  strip_4.show();

  delay(50);
}
void drums() {
    for(int i=0; i<LED_COUNT_1; i++)
  {
    strip_1.setPixelColor(i, 128, 0, 0);
    strip_1.show();
    delay(20);
  }
  for(int i=0; i<LED_COUNT_1; i++)
  {
    strip_1.setPixelColor(i, 0, 128, 0);
    strip_1.show();
    delay(20);
  }
  for(int i=0; i<LED_COUNT_1; i++)
  {
    strip_1.setPixelColor(i, 0, 0, 128);
    strip_1.show();
    delay(20);
  }
  for(int i=0; i<LED_COUNT_1; i++)
  {
    strip_1.setPixelColor(i, 0, 0, 0);
    strip_1.show();
    delay(20);
  }

  for(int i=0; i<LED_COUNT_2; i++)
  {
    strip_2.setPixelColor(i, 128, 0, 0);
    strip_2.show();
    delay(20);
  }
  for(int i=0; i<LED_COUNT_2; i++)
  {
    strip_2.setPixelColor(i, 0, 128, 0);
    strip_2.show();
    delay(20);
  }
  for(int i=0; i<LED_COUNT_2; i++)
  {
    strip_2.setPixelColor(i, 0, 0, 128);
    strip_2.show();
    delay(20);
  }
  for(int i=0; i<LED_COUNT_2; i++)
  {
    strip_2.setPixelColor(i, 0, 0, 0);
    strip_2.show();
    delay(20);
  }

  for(int i=0; i<LED_COUNT_1; i++)
  {
    strip_3.setPixelColor(i, 128, 0, 0);
    strip_3.show();
    delay(20);
  }
  for(int i=0; i<LED_COUNT_1; i++)
  {
    strip_3.setPixelColor(i, 0, 128, 0);
    strip_3.show();
    delay(20);
  }
  for(int i=0; i<LED_COUNT_1; i++)
  {
    strip_3.setPixelColor(i, 0, 0, 128);
    strip_3.show();
    delay(20);
  }
  for(int i=0; i<LED_COUNT_1; i++)
  {
    strip_3.setPixelColor(i, 0, 0, 0);
    strip_3.show();
    delay(20);
  }
}
