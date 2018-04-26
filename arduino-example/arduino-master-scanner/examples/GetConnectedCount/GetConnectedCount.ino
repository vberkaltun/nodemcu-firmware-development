#include <Wire.h>
#include <MasterScanner.h>

void setup() {
  Wire.begin(D1, D2);
  Serial.begin(9600);
  MasterScanner.onConnectedSlaves(connectedSlaves);
  MasterScanner.onDisconnectedSlaves(disconnectedSlaves);
}

void loop() {
  MasterScanner.scanSlaves();
}

void connectedSlaves(uint8_t _array[], byte _count) {
  Serial.print("Done! " + (String)MasterScanner.getConnectedSlavesCount() +
               " is device(s) connected to I2C bus.\n");
}

void disconnectedSlaves(uint8_t _array[], byte _count) {
  Serial.print("Done! " + (String)MasterScanner.getConnectedSlavesCount() +
               " is device(s) connected to I2C bus.\n");
}
