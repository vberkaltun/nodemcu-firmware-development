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
  Serial.print("0x" + (String)_array[0] + " is connected to I2S bus? (0/1) " +
               (String)MasterScanner.isConnected(_array[0]) + ".\n");
}

void disconnectedSlaves(uint8_t _array[], byte _count) {
  Serial.print("0x" + (String)_array[0] + " is connected to I2S bus? (0/1) " +
               (String)MasterScanner.isConnected(_array[0]) + ".\n");
}
