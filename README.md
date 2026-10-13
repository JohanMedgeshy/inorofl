#include <Wire.h>
void setup() {
  Serial.begin(9600);
  Wire.begin();
  for (byte a = 1; a < 127; a++) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission() == 0) {
      Serial.print("Найдено: 0x");
      Serial.println(a, HEX);
    }
  }
}
void loop() {}
