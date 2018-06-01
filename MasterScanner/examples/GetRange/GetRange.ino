#include <Wire.h>
#include <MasterScanner.h>

#define START_ADDRESS 0x10
#define STOP_ADDRESS 0x70
#define INTERVAL 1500

void setup() {
  Wire.begin();
  Serial.begin(9600);
  MasterScanner.setRange(INTERVAL, START_ADDRESS, STOP_ADDRESS);
  Serial.print("Scanning will begin from " + (String)MasterScanner.getStartAddress() +
               " to " + (String)MasterScanner.getStopAddress() +
               " with " + (String)MasterScanner.getIntervalMillis() + " intervals.\n");
}

void loop() {
  MasterScanner.scanSlaves();
}
