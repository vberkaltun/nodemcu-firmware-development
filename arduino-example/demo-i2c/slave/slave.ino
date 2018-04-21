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

// In sequence, delimiters is 0x18, 0x1B and 0x1C - 0x1D
unsigned short sizeofProtocolDelimiters = 3;
const char* protocolDelimiters = "";

unsigned short sizeofDataDelimiters = 1;
const char* dataDelimiters = "";

const char nullIdle = 0x1E;
const char sectionIdle = 0x1F;

const char singleStartIdle = 0x15;
const char multiStartIdle = 0x16;
const char multiEndIdle = 0x17;

const char sizeofFunctionList = 2;
const char* functionList[] = {"getVendors",
                              "getFunctionList"
                             };

// Do not change default value of this variable
char* receivedBuffer = NULL;
unsigned short sizeofReceivedBuffer = 0;

// Do not change default value of this variable
char** givenBuffer = NULL;
unsigned short sizeofGivenBuffer = 0;
unsigned short indexofGivenBuffer = 0;

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

void receiveEvent() {

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
  Serial.print("Error! Unexpected <");
  Serial.print(data);
  Serial.print(">[");
  Serial.print(sizeofData);
  Serial.println("] data received.");

  Serial.println(freeMemory());
}

void executeEvent(unsigned short sizeofData, char data[]) {

  // Store index of found function
  char foundIndex = 0;
  bool foundFlag = true;

  // Decode given data and store it
  char **insideData = Serialization.decode(dataDelimiters, data);

  for (char index = 0; index < functionListSize; index++) {

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
    foundFlag = true;

    for (unsigned short subIndex = 0; subIndex < sizeofInternalFunction; subIndex++) {
      if (insideData[0][subIndex] != functionList[index][subIndex]) {
        foundFlag = false;
        break;
      }
    }

    // If a function found in function list,
    // Store index of this function and break current loop
    if (foundFlag) {
      foundIndex = index;
      break;
    }
  }

  // In the worst case, we can not find a function and found index is -1
  if (foundFlag)
    callReceivedFunction(foundIndex, insideData[1]);
  else
    unknownEvent(sizeofData, data);
  Serial.println(freeMemory());
}

void callReceivedFunction(unsigned short index, char data[]) {

  Serial.println(data);
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

  // Malloc and realloc a sentence,  a list of words
  if (receivedBuffer == NULL)
    receivedBuffer = (char *) malloc(sizeof (char) * (sizeofReceivedBuffer + sizeofNewReceivedBuffer + 1));
  else
    receivedBuffer = (char *) realloc(receivedBuffer, sizeof (char) * (sizeofReceivedBuffer + sizeofNewReceivedBuffer + 1));

  for (unsigned short index = 0; index < sizeofNewReceivedBuffer; index++)
    receivedBuffer[sizeofReceivedBuffer + index] = newReceivedBuffer[0][index];

  // Add an endofline character to tail
  receivedBuffer[sizeofReceivedBuffer + sizeofNewReceivedBuffer] = '\0';
  sizeofReceivedBuffer += sizeofNewReceivedBuffer;

  // -----

  // Go to next step, decoding inside data
  if (newReceivedBuffer[1][0] == (char)singleStartIdle || newReceivedBuffer[1][0] == (char)multiEndIdle) {
    executeEvent(sizeofReceivedBuffer, receivedBuffer);

    // At the end, free up out-of-date buffer data
    free(receivedBuffer);
    receivedBuffer = NULL;
    sizeofReceivedBuffer = 0;
  }

  // If everything goes well, we will arrive here and return true
  return true;
}

bool encodeData(unsigned short sizeofData, char data[]) {

  if (sizeofData == 0 || data == NULL)
    return false;

  // Second, calculete size of data and modulus
  unsigned short modulusofGivenBuffer = (strlen(data) % DIVISOR_NUMBER);
  sizeofGivenBuffer = (strlen(data) / DIVISOR_NUMBER);

  // Add modulos of data, if possible
  if (modulusofGivenBuffer > 0)
    sizeofGivenBuffer++;

  // Malloc and realloc a sentence,  a list of words
  if (givenBuffer == NULL)
    givenBuffer = (char **) malloc(sizeof (char *) * (sizeofGivenBuffer + 1));
  else
    givenBuffer = (char **) realloc(givenBuffer, sizeof (char *) * (sizeofGivenBuffer + 1));

  // -----

  for (unsigned short index = 0; index < sizeofGivenBuffer; index++) {

    givenBuffer[index] = (char *) malloc(sizeof (char) * (DIVISOR_NUMBER + sizeofProtocolDelimiters + 1));

    unsigned short subIndex;
    unsigned short upperBound = (index == sizeofGivenBuffer - 1 ? modulusofGivenBuffer : DIVISOR_NUMBER);

    for (subIndex = 0; subIndex < upperBound; subIndex++)
      givenBuffer[index][subIndex + 1] = data[(index * upperBound) + subIndex];

    // IMPORTANT NOTICE: At the here, We have two status for encoding data(s)
    // If you set the penultimate char as multiSTART, this means data is still available
    // For encoding. But if you set this var as multiEND, this means encoding is over
    // We are making this for receiver side. singleSTART means that data can encode
    // As one packet, do not need any more encoding
    if (sizeofGivenBuffer == 1)
      givenBuffer[index][subIndex + 2] = (char)singleStartIdle;
    else if (index == sizeofGivenBuffer - 1)
      givenBuffer[index][subIndex + 2] = (char)multiEndIdle;
    else
      givenBuffer[index][subIndex + 2] = (char)multiStartIdle;

    givenBuffer[index][0] = protocolDelimiters[0];
    givenBuffer[index][subIndex + 1] = protocolDelimiters[1];
    givenBuffer[index][subIndex + 3] = protocolDelimiters[2];
    givenBuffer[index][subIndex + 4] = '\0';
  }

  // Do not forget to end-of-line char to tail of 2D array
  givenBuffer[sizeofGivenBuffer] = '\0';

  // If everything goes well, we will arrive here and return true
  return true;
}

bool isNumeric(unsigned short sizeofData, char data[]) {

  if (sizeofData == 0 || data == NULL)
    return false;

  for (unsigned short index = 0; index < sizeofData; index++)
    if (isdigit(data[index]) == 0)
      return false;

  return true;
}

bool isAlphanumeric(unsigned short sizeofData, char data[]) {

  if (sizeofData == 0 || data == NULL)
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

void getVendors() {

  char inlineDelimiter[] = {sectionIdle, sectionIdle};
  char* inlineData[] = {DEVICE_BRAND, DEVICE_MODEL, DEVICE_VERSION};
  char* outputData = Serialization.encode(2, inlineDelimiter, 3, inlineData);

  Serial.println(outputData);
}

void getFunctionList() {

  char inlineDelimiter[] = {sectionIdle};
  char* outputData = Serialization.encode(1, inlineDelimiter, sizeofFunctionList, functionList);
  Serial.println(outputData);
}
