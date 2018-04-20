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

void receiveEvent(int sizeofData) {

  // Declare a new variable about given size
  char *newReceivedBuffer;
  int newReceivedIndex = 0;

  // Loop through all but the last
  while (Wire.available()) {

    // receive byte as a character
    char data = Wire.read();

    // Malloc and realloc a sentence,  a list of words
    if (newReceivedBuffer == NULL)
      newReceivedBuffer = (char *) malloc(sizeof (char) * (++newReceivedIndex + 1));
    else
      newReceivedBuffer = (char *) realloc(newReceivedBuffer, sizeof (char) * (++newReceivedIndex + 1));

    newReceivedBuffer[newReceivedIndex - 1] = data;
    newReceivedBuffer[newReceivedIndex ] = '\0';

    // We found singleStartIdle character, break it
    if (newReceivedBuffer[newReceivedIndex - 1] == (char)protocolEndIdle)
      break;
  }

  // -----

  if (!decodeData(newReceivedIndex, newReceivedBuffer))
    unknownEvent(newReceivedIndex, newReceivedBuffer);

  // At the end, free up out-of-date buffer data
  free(receivedBuffer);
  receivedBuffer = NULL;
  receivedBufferSize = 0;

  free(newReceivedBuffer);
}

void requestEvent() {

  // Fetch first stored data on givenList
  char *subGivenData = popGivenBuffer();

  if (subGivenData != NULL)
    Wire.write(subGivenData);
  else {

    // When given data is NULL, add an value as NULL
    encodeData("NULL");

    // ... And pop that to I2C bus
    Wire.write(popGivenBuffer());
  }
}

void unknownEvent(int sizeofData, char data[]) {

  // Notify user
  Serial.println("Error!");

  Serial.print("Unexpected <");
  Serial.print(data);
  Serial.print(">[");
  Serial.print(sizeofData);
  Serial.println("] data received.");

  Serial.println(freeMemory());
}

void executeEvent(int sizeofData, char data[]) {

  // Store index of found function
  int foundIndex = -1;

  // Decode given data and store it
  char **insideData = Serialization.decode(dataDelimiters, data);

  for (int index = 0; index < functionListSize; index++) {

    // Null operator check
    if (insideData == NULL)
      break;

    // Null operator check
    if (insideData[0] == NULL || insideData[1] == NULL)
      break;

    // -----

    // Calculate registered function length at the given index
    int sizeofInternalFunction = strlen(functionList[index]);

    // Calculate registered function length at the given index
    int sizeofExternalFunction = strlen(insideData[0]);

    // Data size check
    if (sizeofInternalFunction != sizeofExternalFunction)
      break;

    // -----

    // Found status flag, using for to find operate
    bool foundFlag = true;

    for (int subIndex = 0; subIndex < sizeofInternalFunction; subIndex++) {
      if (insideData[0][subIndex] != functionList[index][subIndex]) {
        foundFlag = false;
        break;
      }
    }

    // If a function found in function list,
    // Store index of this function and break loop
    if (foundFlag) {
      foundIndex = index;
      break;
    }
  }
  
  // At the end, free up out-of-date buffer data
  free(insideData[0]);
  free(insideData[1]);
  insideData = NULL;

  Serial.println(freeMemory());
}

