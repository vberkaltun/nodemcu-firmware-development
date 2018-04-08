#include <Wire.h>

void setup() {
  Wire.begin(8);                          // join i2c bus with address #8
  Wire.onReceive(receiveEvent);           // register event
  Wire.onRequest(requestEvent);           // register event
  Serial.begin(9600);                     // start serial for output
}

void loop() {
}

void receiveEvent(int howMany) {
  while (Wire.available()) {              // loop through all but the last
    char c = Wire.read();                 // receive byte as a character
    Serial.print(c);                      // print the character
    if (c == '\n') break;
  }
}

void requestEvent() {
  Wire.write("Hello master!\n     This will not appear!");
}
