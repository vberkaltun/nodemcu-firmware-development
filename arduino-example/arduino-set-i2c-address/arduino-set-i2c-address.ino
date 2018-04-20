#include <EEPROM.h>

// IMPORTANT NOTICE: Do not change this variable
// Thus, 0x03 index was reserved for eeprom address of device
#define EEPROM_ADDRESS 0x03

// Device address, only change there
const int deviceAddress = 0x01;

void setup() {

  // Initialize serial and wait for port to open:
  Serial.begin(9600);

  // Notify of erase...
  Serial.print("Initializing... ");

  // Clear all eeprom addres on device
  for (int index = 0 ; index < EEPROM.length() ; index++)
    EEPROM.write(index, 0);

  // Notify of init...
  Serial.println("OK.");
  delay(1000);

  // Notify of erase...
  Serial.println("All EEPROM addresses was cleared successfully.");
  delay(1000);
  
  // -----

  // Set ours new device address to address field
  EEPROM.write(EEPROM_ADDRESS, deviceAddress);

  // Done! Notify user
  Serial.print("Device address [");
  Serial.print("0x");
  Serial.print(EEPROM.read(EEPROM_ADDRESS), HEX);
  Serial.print("] was setted to given eeprom address [");
  Serial.print("0x");
  Serial.print(EEPROM_ADDRESS, HEX);
  Serial.println("].");
}

void loop() {
}
