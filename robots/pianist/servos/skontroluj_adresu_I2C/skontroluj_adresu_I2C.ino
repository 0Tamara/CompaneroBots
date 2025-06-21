#include <Wire.h>
void setup() {
  Serial.begin(115200);
  Wire.begin();
  Serial.println("I2C Scanner");
  for (uint8_t addr = 0x00; addr <= 0x7F; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("Zariadenie na adrese: 0x");
      if (addr < 16) Serial.print("0");
      Serial.println(addr, HEX);
    }
  }
}