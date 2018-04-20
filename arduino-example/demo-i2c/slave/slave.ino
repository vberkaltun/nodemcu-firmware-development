#include <Wire.h>
#include <Serializer.h>
#include <EEPROM.h>
#include <MemoryFree.h>

// IMPORTANT NOTICE: These all constant is depending on your protocol
// As you can see, this protocol delimiter was declared in this scope
// That's mean, all function will use this delimiter constant on
// Communication between two or more devices
#define DEVICE_BRAND "Broadcom_Inc."
#define DEVICE_MODEL "BCM_43142"
#define DEVICE_VERSION "VER_1.0.0"

// IMPORTANT NOTICE: Do not change this variable
// Thus, 0x03 index was reserved for bus address of device
#define EEPROM_ADDRESS 0x03

// IMPORTANT NOTICE: On I2C bus, You can send up to 32 bits on
// Each transmission. Therefore, if there is more data than 32 bits
// We should split it. Then we can send our data to master
#define DIVISOR_NUMBER 25

// In sequence, delimiters is 0x1C, 0x1D and 0x1B - 0x1E
const char* protocolDelimiters = "";
const char* dataDelimiters = "";

const char singleStartIdle = 0x1E;
const char multiStartIdle = 0x1A;
const char multiEndIdle = 0x16;
const char protocolEndIdle = 0x1B;

const int functionListSize = 2;
const char* functionList[] = {"getVendors",
                              "getFunctionList"
                             };

// Do not change default value of this variable
char* receivedBuffer = NULL;
int receivedBufferSize = 0;

// Do not change default value of this variable
char** givenBuffer = NULL;
int givenBufferSize = 0;
int givenBufferIndex = 0;

void setup() {

  // Register events, onReceive and onRequest
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

  // Initialize communication on serial protocol
  Serial.begin(9600);

  // Initialize communication on Wire protocol
  Wire.begin(EEPROM.read(EEPROM_ADDRESS));
}

void loop() {
}

