#include <Wire.h>
#include <MasterScanner.h>

#define START_ADDRESS 0x30
#define STOP_ADDRESS 0x77
#define INTERVAL 3000

void setup() {
  Wire.begin();
  Serial.begin(9600);
  MasterScanner.setRange(INTERVAL, START_ADDRESS, STOP_ADDRESS);
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
