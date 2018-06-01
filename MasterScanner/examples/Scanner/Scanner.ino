#include <Wire.h>
#include <MasterScanner.h>

void setup() {
  Wire.begin();
  Serial.begin(9600);
  MasterScanner.onConnectedSlaves(connectedSlaves);
  MasterScanner.onDisconnectedSlaves(disconnectedSlaves);
}

void loop() {
  MasterScanner.scanSlaves();
}

void connectedSlaves(uint8_t _array[], byte _count) {

  Serial.print("Done! ");
  Serial.print(_count, DEC);
  Serial.println(" slave device(s) connected to I2C bus.");

  Serial.print("ID(s): ");
  for (int i = 0; i < _count; i++) {
    Serial.print("0x");
    Serial.print(_array[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void disconnectedSlaves(uint8_t _array[], byte _count) {

  Serial.print("Done! ");
  Serial.print(_count, DEC);
  Serial.println(" slave device(s) disconnected from I2C bus.");

  Serial.print("ID(s): ");
  for (int i = 0; i < _count; i++) {
    Serial.print("0x");
    Serial.print(_array[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}
