#include <Wire.h>
#define  SLAVE_ADDRESS           0x09  //slave address,any number from 0x01 to 0x7F

void setup()
{
  Wire.begin(SLAVE_ADDRESS);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
}

void loop() {
}

void requestEvent() {
}

void receiveEvent(int bytesReceived) {
}
