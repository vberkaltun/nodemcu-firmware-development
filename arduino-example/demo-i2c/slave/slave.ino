#include <Wire.h>
#include <Serializer.h>
#include <EEPROM.h>
#include <MemoryFree.h>

// IMPORTANT NOTICE: These all constant is depending on your protocol
// As you can see, this protocol delimiter was declared in this scope
// That's mean, all function will use this delimiter constant on
// Communication between two or more devices
#define DEVICE_BRAND "Broadcom_Incorporated"
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

void receiveEvent(unsigned short sizeofData) {

  // Declare variables about given size
  char *newReceivedBuffer = (char *) malloc(sizeof (char) * (sizeofData + 1));
  unsigned short indexofNewReceivedBuffer = 0;

  // Loop through all but the last
  while (Wire.available())
    newReceivedBuffer[indexofNewReceivedBuffer++] =  Wire.read();
  newReceivedBuffer[sizeofData] = '\0';

  // -----

  if (!decodeData(indexofNewReceivedBuffer, newReceivedBuffer))
    unknownEvent(indexofNewReceivedBuffer, newReceivedBuffer);

  // TEMPORARILY, will be delete on release
  Serial.println(freeMemory());

  free(newReceivedBuffer);
}

void requestEvent() {

  // IMPORTANT NOTICE: On the worst case, we are sending empty protocol
  // Data(s) this is the best way of worst case because the receiver device
  // Can always Decode this data and when we send "not filled" protocol
  // Data(s), this means we are sending empty data(s)
  if (indexofGivenBuffer >= sizeofGivenBuffer)
    Wire.write(protocolDelimiters);
  else {
    if (sizeofGivenBuffer > 0)
      Wire.write(givenBuffer[indexofGivenBuffer++]);
    else
      Wire.write(protocolDelimiters);
  }
}

void unknownEvent(unsigned short sizeofData, char data[]) {

  // Notify user
  Serial.print("Error! Unexpected <");
  Serial.print(data);
  Serial.print(">[");
  Serial.print(sizeofData);
  Serial.println("] data received.");

  // At the end, free up out-of-date buffer data
  free(receivedBuffer);
  receivedBuffer = NULL;
  sizeofReceivedBuffer = 0;
}

void executeEvent(unsigned short sizeofData, char data[]) {

  // Store index of found function
  char foundIndex = 0;
  bool foundFlag = false;

  // Decode given data and store it
  char **insideData = Serialization.decode(sizeofDataDelimiters, dataDelimiters, sizeofData, data);

  for (char index = 0; index < sizeofFunctionList; index++) {

    // Null operator check
    if (insideData == NULL)
      break;

    // Null operator check
    if (insideData[0] == NULL || insideData[1] == NULL)
      break;

    // Calculate registered function length at the given index
    unsigned short sizeofInternalFunction = strlen(functionList[index]);
    unsigned short sizeofExternalFunction = strlen(insideData[0]);

    // Data size check
    if (sizeofInternalFunction != sizeofExternalFunction)
      continue;

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
    callReceivedFunction(foundIndex, strlen(insideData[1]), insideData[1]);
  else
    unknownEvent(sizeofData, data);

  // At the end, free up out-of-date buffer data
  free(receivedBuffer);
  receivedBuffer = NULL;
  sizeofReceivedBuffer = 0;
}

bool decodeData(unsigned short sizeofData, char data[]) {

  if (sizeofData == 0 || data == NULL)
    return false;

  // Decode given data, Calculate up-of-date and needed buffer size
  char **newReceivedBuffer = Serialization.decode(sizeofProtocolDelimiters, protocolDelimiters, sizeofData, data);
  unsigned short sizeofNewReceivedBuffer = strlen(newReceivedBuffer[0]);

  // Null operator check
  if (newReceivedBuffer == NULL)
    return false;

  // Null operator check
  if (newReceivedBuffer[0] == NULL || newReceivedBuffer[1] == NULL)
    return false;

  // -----

  switch (newReceivedBuffer[1][0]) {
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
  if (newReceivedBuffer[1][0] == (char)singleStartIdle || newReceivedBuffer[1][0] == (char)multiEndIdle)
    executeEvent(sizeofReceivedBuffer, receivedBuffer);

  // If everything goes well, we will arrive here and return true
  return true;
}

bool encodeData(unsigned short sizeofData, char data[]) {

  if (sizeofData == 0 || data == NULL)
    return false;

  // Second, calculete size of data and modulus
  unsigned short modulusofGivenBuffer = (sizeofData % DIVISOR_NUMBER);
  sizeofGivenBuffer = (sizeofData / DIVISOR_NUMBER);

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
      givenBuffer[index][subIndex + 1] = data[(index * DIVISOR_NUMBER) + subIndex];

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

void callReceivedFunction(unsigned short indexofFunction, unsigned short sizeofData, char data[]) {

  // IMPORTANT NOTICE: Before the calling internal functions,
  // Last stored data must be removed on memory. Otherwise, we can not sent
  // Last stored data to master device. And additional, data removing will refresh
  // the size of data in memory. This is most important thing ...
  for (char index = 0; index < sizeofGivenBuffer; index++)
    free(givenBuffer[index]);

  // After free up top level pointer
  free(givenBuffer);
  givenBuffer = NULL;
  sizeofGivenBuffer = 0;
  indexofGivenBuffer = 0;

  switch (indexofFunction) {
    case 0:
      getVendors(sizeofData, data);
      break;
    case 1:
      getFunctionList(sizeofData, data);
      break;
    default:
      break;
  }
}

// ------------------------------------------------------------
// ----- INTERNAL FUNC(S) -------------------------------------
// ------------------------------------------------------------

void getVendors(unsigned short sizeofData, char data[]) {

  // Notify users
  Serial.print("Done! ");
  Serial.print(__func__);
  Serial.println(" function triggered.");

  char* deviceData[] = {DEVICE_BRAND, DEVICE_MODEL, DEVICE_VERSION};
  char* result = Serialization.encode(2, "", 3, deviceData);
  encodeData(strlen(result), result);
}

void getFunctionList(unsigned short sizeofData, char data[]) {

  // Notify users
  Serial.print("Done! ");
  Serial.print(__func__);
  Serial.println(" function triggered.");

  char *result = Serialization.encode(1, nullIdle, sizeofFunctionList, functionList);
  encodeData(strlen(result), result);
}
