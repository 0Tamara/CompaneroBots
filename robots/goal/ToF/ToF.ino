#include <VL53L0X.h>

#define THR_DISTANCE 1000

VL53L0X distanceSensor;
float vzdialenost;
unsigned long cas;
bool bezi = false;
bool vypis = false;

void setup() {
  Serial.begin(115200);
  pinMode(2,OUTPUT);

  Wire.begin();
  distanceSensor.setTimeout(100);
  if (!distanceSensor.init())
  {
    Serial.println("Failed to detect and initialize sensor VL53L0X !");
  }
  Serial.println("Sensor VL53L0X online!");
}

void loop() {
  vzdialenost = (float)distanceSensor.readRangeSingleMillimeters();
  if(vzdialenost < THR_DISTANCE)
  {
    Serial.println(vzdialenost);
    digitalWrite(2, HIGH);
    delay(100);
    digitalWrite(2, LOW);
    delay(500);
  }
}