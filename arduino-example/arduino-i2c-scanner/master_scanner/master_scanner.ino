#include <Wire.h>
#include <i2cdetect.h>

const uint8_t first = 0x01;
const uint8_t last = 0x7F;

void setup() {
  Wire.begin();
  Serial.begin(9600);
  Serial.print("i2cdetect example \n");
  Serial.print("Scanning address range 0x01 - 0x7F \n");
}

void loop() {
  i2cdetect(first, last);
  delay(2000);
}
