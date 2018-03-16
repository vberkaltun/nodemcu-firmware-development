/*
  ESP8266 Blink by Berk Altun
  Blink the blue and other GPIO LEDs on the ESP-01 module
*/

// D0 - D10 arası gerekli "Digital GPIO No" larını saklayan kutuphane, zorunlu degil
#include <pins_arduino.h>

// INT dizilerde Dogrudan Dx kodlamasi ile kullanmak mumkun
uint8_t  Pins[] = {D0, D1, D2, D3, D4, D5, D6, D7, D8, D9, D10};

// Bu kullanim cok fazla hata iceriyor, Dx kullanimini tercih et
// uint8_t  Pins[] = {16, 5, 4, 0, 2, 14, 12, 13, 15};

const int DELAY_SIZE = 10;

void setup() {
  for (int thisPin = 0; thisPin < sizeof(Pins); thisPin++) {
    pinMode(Pins[thisPin], OUTPUT);
    digitalWrite(Pins[thisPin], LOW);
  }
}

void loop() {

  for (int thisPin = 0; thisPin < sizeof(Pins); thisPin++) {
    digitalWrite(Pins[thisPin], HIGH);
    delay(DELAY_SIZE);// Wait for a second
  }

  for (int thisPin = 0; thisPin < sizeof(Pins); thisPin++) {
    digitalWrite(Pins[thisPin], LOW);
    delay(DELAY_SIZE);// Wait for a second
  }
}
