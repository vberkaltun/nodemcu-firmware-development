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

const char functionListSize = 2;
const char* functionList[] = {"getVendors",
                              "getFunctionList"
                             };

// Do not change default value of this variable
char* receivedBuffer = NULL;
unsigned short receivedBufferSize = 0;

// Do not change default value of this variable
char** givenBuffer = NULL;
unsigned short givenBufferSize = 0;
unsigned short givenBufferIndex = 0;

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
  unsigned short newReceivedIndex = 0;

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

void unknownEvent(unsigned short sizeofData, char data[]) {

  // Notify user
  Serial.println("Error!");

  Serial.print("Unexpected <");
  Serial.print(data);
  Serial.print(">[");
  Serial.print(sizeofData);
  Serial.println("] data received.");

  Serial.println(freeMemory());
}

void executeEvent(unsigned short sizeofData, char data[]) {

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
    unsigned short sizeofInternalFunction = strlen(functionList[index]);

    // Calculate registered function length at the given index
    unsigned short sizeofExternalFunction = strlen(insideData[0]);

    // Data size check
    if (sizeofInternalFunction != sizeofExternalFunction)
      break;

    // -----

    // Found status flag, using for to find operate
    bool foundFlag = true;

    for (unsigned short subIndex = 0; subIndex < sizeofInternalFunction; subIndex++) {
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

void callReceivedFunction(unsigned short index, char data[]) {

  Serial.print(data);
}

char* popGivenBuffer() {

  if (givenBuffer == NULL)
    return NULL;

  if (givenBufferIndex >= givenBufferSize) {

    for (unsigned short index = 0; index < givenBufferSize; index++)
      free(givenBuffer[index]);

    free(givenBuffer);
    givenBuffer = NULL;
    givenBufferIndex = 0;

    return NULL;
  }

  return givenBuffer[givenBufferIndex++];
}

bool decodeData(unsigned short sizeofData, char data[]) {

  // Decode given data and store it
  char **newReceivedBuffer = Serialization.decode(protocolDelimiters, data);

  // Null operator check
  if (newReceivedBuffer == NULL)
    return false;

  // Null operator check
  if (newReceivedBuffer[0] == NULL || newReceivedBuffer[1] == NULL)
    return false;

  switch (newReceivedBuffer[1][0] ) {
    case singleStartIdle:
    case multiStartIdle:
    case multiEndIdle:
      break;
    default:
      return false;
  }

  // -----

  // Calculate up-of-date and needed buffer size
  unsigned short newReceivedSize = strlen(newReceivedBuffer[0]);

  // Malloc and realloc a sentence,  a list of words
  if (receivedBuffer == NULL)
    receivedBuffer = (char *) malloc(sizeof (char) * (receivedBufferSize + newReceivedSize + 1));
  else
    receivedBuffer = (char *) realloc(receivedBuffer, sizeof (char) * (receivedBufferSize + newReceivedSize + 1));

  for (unsigned short index = 0; index < newReceivedSize; index++)
    receivedBuffer[receivedBufferSize + index] = newReceivedBuffer[0][index];

  // Add an endofline character to tail
  receivedBuffer[receivedBufferSize + newReceivedSize] = '\0';
  receivedBufferSize += newReceivedSize;

  // -----

  // Go to next step, decoding inside data
  if (newReceivedBuffer[1][0] == (char)singleStartIdle || newReceivedBuffer[1][0] == (char)multiEndIdle)
    executeEvent(receivedBufferSize, receivedBuffer);

  // If everything goes well, we will arrive here and return true
  return true;
}

bool encodeData(char data[]) {

  if (data == NULL)
    return false;

  // First, calculete size of delimiters and store it
  unsigned short delimiterSize = strlen(protocolDelimiters) + 1;

  // Second, calculete size of data and store it
  givenBufferSize = (strlen(data) / DIVISOR_NUMBER);

  if ((strlen(data) % DIVISOR_NUMBER) > 0)
    givenBufferSize++;

  // Malloc and realloc a sentence,  a list of words
  if (givenBuffer == NULL)
    givenBuffer = (char **) malloc(sizeof (char *) * (givenBufferSize + 1));
  else
    givenBuffer = (char **) realloc(givenBuffer, sizeof (char *) * (givenBufferSize + 1));

  // -----

  for (unsigned short index = 0; index < givenBufferSize; index++) {

    givenBuffer[index] = (char *) malloc(sizeof (char) * (DIVISOR_NUMBER + delimiterSize + 1));
    givenBuffer[index][0] = protocolDelimiters[0];

    for (unsigned short subIndex = 0;; subIndex++) {

      if (data[(index * DIVISOR_NUMBER) + subIndex] == '\0' || subIndex == DIVISOR_NUMBER) {

        givenBuffer[index][subIndex + 1] = protocolDelimiters[1];

        // IMPORTANT NOTICE: At the here, We have two status for encoding data(s)
        // If you set the penultimate char as multiSTART, this means data is still available
        // For encoding. But if you set this var as multiEND, this means encoding is over
        // We are making this for receiver side. singleSTART means that data can encode
        // As one packet, do not need any more encoding
        if (givenBufferSize == 1)
          givenBuffer[index][subIndex + 2] = (char)singleStartIdle;
        else if (index == givenBufferSize - 1)
          givenBuffer[index][subIndex + 2] = (char)multiEndIdle;
        else
          givenBuffer[index][subIndex + 2] = (char)multiStartIdle;

        givenBuffer[index][subIndex + 3] = protocolDelimiters[2];
        givenBuffer[index][subIndex + 4] = '\0';

        break;
      }

      givenBuffer[index][subIndex + 1] = data[(index * DIVISOR_NUMBER) + subIndex];
    }
  }

  // Do not forget to end-of-line char to tail of 2D array
  givenBuffer[givenBufferSize] = '\0';

  // If everything goes well, we will arrive here and return true
  return true;
}

bool isNumeric(unsigned short sizeofData, char data[]) {

  if (data == NULL)
    return false;

  for (unsigned short index = 0; index < sizeofData; index++)
    if (isdigit(data[index]) == 0)
      return false;

  return true;
}

bool isAlphanumeric(unsigned short sizeofData, char data[]) {

  if (data == NULL)
    return false;

  // Check function ID, type is alphanumeric
  for (unsigned short index = 0; index < sizeofData; index++)
    if (isalnum(data[index]) == 0)
      return false;

  return true;
}

// ------------------------------------------------------------
// ----- INTERNAL FUNC(S) -------------------------------------
// ------------------------------------------------------------

