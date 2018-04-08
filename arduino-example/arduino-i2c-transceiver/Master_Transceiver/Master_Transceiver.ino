#include <Wire.h>

void setup() {
  Wire.begin();                           // join i2c bus (address optional for master)
  Serial.begin(9600);                     // start serial for output
}


void loop() {

  Wire.beginTransmission(8);              // transmit to device #8
  Wire.write("Hello slave!\n      This will not appear!");
  Wire.endTransmission();                 // stop transmitting

  Wire.requestFrom(8, 120);               // request 6 bytes from slave device #8
  
  while (Wire.available()) {              // loop through all but the last
    char c = Wire.read();                 // receive byte as a character
    Serial.print(c);                      // print the character
    if (c == '\n') break;
  }
  
  delay(1000);
}
