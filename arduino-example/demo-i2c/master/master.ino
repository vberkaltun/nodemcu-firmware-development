#include <Wire.h>
#include <Serializer.h>
#include <QueueList.h>
#include <MasterScanner.h>

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

// IMPORTANT NOTICE: Based on buffer size of I2C bus and maximum
// Range of your device. At the here, we declared it with 32 bit
// Because of buffer size of Arduino but if you have a bigger buffer
// Than 32 bit. you can upgrade and speed up your buffers
#define BUFFER_SIZE 32

// IMPORTANT NOTICE: IMPORTANT NOTICE: If buffer size is not looking
// Enough for you, you can extend or shrink your data with this variable.
// Due to lack of resources on memory, we were setted it as 8 but if you
// Have more memory on your device, bigger value can be compatible
#define MINIMIZED_BUFFER_SIZE 8

// Outside protocol delimiters
#define PROTOCOL_DELIMITERS ""
#define PROTOCOL_DELIMITERS_SIZE 3

// Inside protocol delimiters, we called it data delimiters
#define DATA_DELIMITER ""
#define DATA_DELIMITER_SIZE 1

// Start and end type of protocol delimiters
#define IDLE_SINGLE_START 0x15
#define IDLE_MULTI_START 0x16
#define IDLE_MULTI_END 0x17

// -----

enum handshakeData {Unknown, Ready};
enum communicationData {Idle, Continue, End};

struct vendorData {
  char Brand[BUFFER_SIZE];
  char Model[BUFFER_SIZE];
  char Version[BUFFER_SIZE];
};

struct functionData {
  char Name[BUFFER_SIZE];
  bool Request = false;
  unsigned short Interval = 0;
};

struct deviceData {
  struct vendorData vendorList;
  QueueList<functionData> functionList;
  enum handshakeData handshake = Unknown;
  char address = NULL;
};

QueueList<deviceData> deviceList;
enum communicationData communicationFlag = Idle;

// -----

// Do not change default value of this variable
char receivedBuffer[BUFFER_SIZE * MINIMIZED_BUFFER_SIZE];
unsigned short sizeofReceivedBuffer = 0;

// Do not change default value of this variable
char givenBuffer[MINIMIZED_BUFFER_SIZE][BUFFER_SIZE];
unsigned short sizeofGivenBuffer = 0;
unsigned short indexofGivenBuffer = 0;

char sizeofFunctionList = 2;
char *functionList[] = {"getVendors",
                        "getFunctionList"
                       };

void setup() {

  // Initialize communication on serial protocol
  Serial.begin(9600);

  // Initialize communication on Wire protocol
  Wire.begin(D1, D2);

  // IMPORTANT NOTICE: Due to our I2C scanner lib, When a new device
  // Connected or disconnected to master, our I2C scanner lib decides
  // Which one is to be triggered
  MasterScanner.onConnectedSlaves(connectedSlaves);
  MasterScanner.onDisconnectedSlaves(disconnectedSlaves);
}

void loop() {

  MasterScanner.scanSlaves();
  delay(500);
}

void connectedSlaves(uint8_t _array[], byte _count) {

  Serial.print("Done! ");
  Serial.print(_count, DEC);
  Serial.println(" slave device(s) connected to I2C bus. ID(s): ");

  for (int i = 0; i < _count; i++) {
    Serial.print("\t ID: 0x");
    Serial.println(_array[i], HEX);
  }
  Serial.println("\t ---");

  // IMPORTANT NOTICE: Device registering is more priority than others
  // Step, When new device(s) were connected to master device, firstly
  // Register these device(s) to system, after continue what you do
  for (int i = 0; i < _count; i++) {

    // Free up out-of-date buffer data
    receivedBuffer[0] = '\0';
    sizeofReceivedBuffer = 0;

    // Register last added device to system
    registerNewDevice(_array[i]);
  }
}

void disconnectedSlaves(uint8_t _array[], byte _count) {

  Serial.print("Done! ");
  Serial.print(_count, DEC);
  Serial.println(" slave device(s) disconnected from I2C bus.");

  for (int i = 0; i < _count; i++) {
    Serial.print("\t ID: 0x");
    Serial.println(_array[i], HEX);
  }
  Serial.println("\t ---");
}

void unknownEvent(unsigned short sizeofData, char data[]) {

  // Notify user
  Serial.print("Error! Unexpected <");
  Serial.print(data);
  Serial.print(">[");
  Serial.print(sizeofData);
  Serial.println("] data received.");

  // At the end, free up out-of-date buffer data
  receivedBuffer[0] = '\0';
  sizeofReceivedBuffer = 0;
}

void registerNewDevice(char address) {

  for (unsigned short index = 0; index < sizeofFunctionList; index++) {

    // Free up out-of-date buffer data
    // We made it twice, This one is for top level clearing
    receivedBuffer[0] = '\0';
    sizeofReceivedBuffer = 0;

    char *encodeDelimiter = generateDelimiterBuffer(DATA_DELIMITER, 1);
    char *resultBuffer[] = {functionList[index], "NULL"};
    char *result = Serialization.encode(1, encodeDelimiter, 2, resultBuffer);
    encodeData(strlen(result), result);

    // Send all remainder data to newly registered slave
    while (indexofGivenBuffer < sizeofGivenBuffer) {
      Wire.beginTransmission(address);
      Wire.write(givenBuffer[indexofGivenBuffer++]);
      Wire.endTransmission();

      // Maybe not need, right?
      delay(10);
    }

    // IMPORTANT NOTICE: Due to decoding of slave device, we need to Wait
    // A little bit. Otherwise, Master device will request data From slave
    // Device too early and slave cannot send it
    delay(100);

    // -----

    // We will do this till decoding will return false
    while (true) {

      // More info was given at inside of this function
      // Actually, that's not worst case
      if (!requestConfigofNewDevice(address))
        break;

      // IMPORTANT NOTICE: At the here, We are making output control.
      // The code that given at above changes global flag output(s). So,
      // For next operations, We need to check this output(s) and in this
      // Way detect our current status
      if (communicationFlag != Continue)
        break;
    }

    // If it is still IDLE, that's mean data is corrupted (Not END)
    if (communicationFlag == Idle) {
      unknownEvent(sizeofReceivedBuffer, receivedBuffer);
      break;
    }

    // IMPORTANT NOTICE: Final point, If we can arrive there, that's mean
    // Bus communication is OK but inside data is unknown. At the next code,
    // We will try to decode our inside data. If we can not do it, we will
    // Add this unknown device as BLOCKED to system
    if (!fillConfigofNewDevice(index, address))
      break;
  }
}

bool requestConfigofNewDevice(char address) {

  unsigned short indexofNewReceivedBuffer = 0;
  char newReceivedBuffer[BUFFER_SIZE];

  // Request data from slave
  Wire.requestFrom(address, BUFFER_SIZE);
  while (Wire.available()) {

    // Store new received data at the here
    char currentBuffer = Wire.read();
    newReceivedBuffer[indexofNewReceivedBuffer++] =  (char)currentBuffer;

    // Don't forget to add end-of-line char to end
    if (currentBuffer == (char)PROTOCOL_DELIMITERS[2]) {
      newReceivedBuffer[indexofNewReceivedBuffer] = '\0';
      break;
    }
  }

  // Decode last given data
  if (!decodeData(indexofNewReceivedBuffer, newReceivedBuffer))
    return false;

  // Maybe not need, right?
  delay(10);

  // IMPORTANT NOTICE: Actually When we arrived this point, we arrived
  // Worst case point even though It was TRUE. If you came there, program will
  // Run till communication flag will be END or IDLE type. Otherwise, this
  // Point is related with CONTINUE status
  return true;
}

bool fillConfigofNewDevice(unsigned short index, char address) {

  // IMPORTANT NOTICE: Before decoding inside data, we need firstly
  // Calculate count of data delimiter. This result gives us that how
  // Many different data included in this inside data
  unsigned short countofCharacter = howManyCharacter(DATA_DELIMITER, sizeofReceivedBuffer, receivedBuffer);

  // Generate a delimiter data and use in with decoding function
  char *decodeDelimiter = generateDelimiterBuffer(DATA_DELIMITER, countofCharacter);
  char **newReceivedBuffer = Serialization.decode(countofCharacter, decodeDelimiter, sizeofReceivedBuffer, receivedBuffer);

  // Null operator check
  if (newReceivedBuffer == NULL)
    return false;

  // IMPORTANT NOTICE: In this program, we have two data type, one of them
  // Is VENDOR data, other one is FUNCTION data. Because of this we have two
  // Statement in switch case
  switch (index) {
    case 0:
      if (!fillVendorData(address, countofCharacter, newReceivedBuffer)) {

        struct deviceData newDeviceData;
        newDeviceData.handshake = Unknown;
        newDeviceData.address = address;

        // Major code for device list
        deviceList.pushFront(newDeviceData);
        unknownEvent(sizeofReceivedBuffer, receivedBuffer);

        return false;
      }

      // Notify users
      Serial.print("Done! The vendors of [0x");
      Serial.print(address, HEX);
      Serial.println("] address were saved successfully to system.");

      Serial.print("\t BRAND: ");
      Serial.println(deviceList[0].vendorList.Brand);
      Serial.print("\t MODEL: ");
      Serial.println(deviceList[0].vendorList.Model);
      Serial.print("\t VERSION: ");
      Serial.println(deviceList[0].vendorList.Version);
      Serial.println("\t ---");

      break;

    case 1:
      if (!fillFunctionData(address, countofCharacter, newReceivedBuffer))
        return false;

      for (unsigned short index = 0; index < deviceList[0].functionList.size(); index++) {
        Serial.print("\t FUNCTION: ");
        Serial.println(deviceList[0].functionList[index].Name);
      }
      Serial.println("\t ---");

      break;

    default:
      return false;
  }

  // If everything goes well, we will arrive here and return true
  return true;
}

bool fillVendorData(char address, unsigned short sizeofData, char **data) {

  // Worst case, if vendor list size is not equal to default vendor list size
  if (sizeofData != 2)
    return false;

  // IMPORTANT NOTICE: When a new device is registered to master,
  // We are decoding all vendor Data at the here. When we are doing
  // All of these also we need temp variable
  struct deviceData newDeviceData;

  // Best case, we have it all
  for (unsigned short index = 0; index < BUFFER_SIZE; index++) {
    if (data[0][index] == NULL) {
      newDeviceData.vendorList.Brand[index] = '\0';
      break;
    }
    newDeviceData.vendorList.Brand[index] = data[0][index];
  }

  for (unsigned short index = 0; index < BUFFER_SIZE; index++) {
    if (data[1][index] == NULL) {
      newDeviceData.vendorList.Model[index] = '\0';
      break;
    }
    newDeviceData.vendorList.Model[index] = data[1][index];
  }

  for (unsigned short index = 0; index < BUFFER_SIZE; index++) {
    if (data[2][index] == NULL) {
      newDeviceData.vendorList.Version[index] = '\0';
      break;
    }
    newDeviceData.vendorList.Version[index] = data[2][index];
  }

  // -----

  // Major code for device list
  deviceList.pushFront(newDeviceData);

  // If everything goes well, we will arrive here and return true
  return true;
}

bool fillFunctionData(char address, unsigned short sizeofData, char **data) {

  // Worst case, function list size is equal to char - 1 size
  if ((sizeofData + 1) % 3 != 0)
    return false;

  for (unsigned short index = 0; index < sizeofData + 1; index += 3) {

    char internalData0[BUFFER_SIZE];
    sprintf(internalData0, "%s", data[index]);
    unsigned short sizeofInternalData0 = strlen(internalData0);
    if (!isAlphanumeric(sizeofInternalData0, internalData0))
      break;

    char internalData1[BUFFER_SIZE];
    sprintf(internalData1, "%s", data[index + 1]);
    unsigned short sizeofInternalData1 = strlen(internalData1);
    if (!isNumeric(sizeofInternalData1, internalData1))
      break;

    char internalData2[BUFFER_SIZE];
    sprintf(internalData2, "%s", data[index + 2]);
    unsigned short sizeofInternalData2 = strlen(internalData2);
    if (!isNumeric(sizeofInternalData2, internalData2))
      break;

    // -----

    // IMPORTANT NOTICE: When a new device is registered to master,
    // We are decoding all function Data at the here. When we are doing
    // All of these also we need temp variable
    struct functionData newFunctionData;

    for (unsigned short subindex = 0; subindex < BUFFER_SIZE; subindex++) {
      if (internalData0[subindex] == NULL) {
        newFunctionData.Name[subindex] = '\0';
        break;
      }
      newFunctionData.Name[subindex] = internalData0[subindex];
    }

    newFunctionData.Request = (bool)atoi(internalData1);
    newFunctionData.Interval = atoi(internalData2);

    // If we can arrive there, we can say function data is ok
    deviceList[0].functionList.pushFront(newFunctionData);
  }

  // I KNOW ... I KNOW, I AM GOD OF EVERYTHING (JOKE)...
  // ADDED AT 26.04.2018 BY BERK ALTUN, LAST MAJOR ADDITION
  deviceList[0].handshake = Ready;
  deviceList[0].address = address;

  // If everything goes well, we will arrive here and return true
  return true;
}

bool decodeData(unsigned short sizeofData, char data[]) {

  if (sizeofData == 0 || data == NULL)
    return false;

  // Decode given data, Calculate up-of-date and needed buffer size
  char **newReceivedBuffer = Serialization.decode(PROTOCOL_DELIMITERS_SIZE, PROTOCOL_DELIMITERS, sizeofData, data);

  // Null operator check
  if (newReceivedBuffer == NULL)
    return false;

  // Null operator check
  if (newReceivedBuffer[0] == NULL || newReceivedBuffer[1] == NULL)
    return false;

  // -----

  // IMPORTANT NOTICE: As I said before, NodeMCU do not support lots of
  // C++ library. This one is one of these, also. At the here, we can not
  // Use directly strlen function. For solving, first we need to get inside data,
  // After we need to calculate size of the given data
  char internalData0[BUFFER_SIZE];
  sprintf(internalData0, "%s", newReceivedBuffer[0]);
  unsigned short sizeofInternalData0 = strlen(internalData0);

  char internalData1[BUFFER_SIZE];
  sprintf(internalData1, "%s", newReceivedBuffer[1]);
  unsigned short sizeofInternalData1 = strlen(internalData1);

  // -----

  switch (internalData1[0]) {
    case IDLE_SINGLE_START:
    case IDLE_MULTI_END:
      communicationFlag = End;
      break;

    case IDLE_MULTI_START:
      communicationFlag = Continue;
      break;

    default:
      communicationFlag = Idle;

      // Free up out-of-date buffer data
      receivedBuffer[0] = '\0';
      sizeofReceivedBuffer = 0;

      return false;
  }

  for (unsigned short index = 0; index < sizeofInternalData0; index++)
    receivedBuffer[sizeofReceivedBuffer + index] = internalData0[index];

  // Add an endofline character to tail
  receivedBuffer[sizeofReceivedBuffer + sizeofInternalData0] = '\0';
  sizeofReceivedBuffer += sizeofInternalData0;

  // If everything goes well, we will arrive here and return true
  return true;
}

bool encodeData(unsigned short sizeofData, char data[]) {

  // IMPORTANT NOTICE: Before the calling internal functions,
  // Last stored data must be removed on memory. Otherwise, we can not sent
  // Last stored data to master device. And additional, data removing will refresh
  // the size of data in memory. This is most important thing ...
  for (char index = 0; index < sizeofGivenBuffer; index++)
    givenBuffer[index][0] = '\0';

  sizeofGivenBuffer = 0;
  indexofGivenBuffer = 0;

  // -----

  if (sizeofData == 0 || data == NULL)
    return false;

  // Second, calculete size of data and modulus
  unsigned short modulusofGivenBuffer = (sizeofData % DIVISOR_NUMBER);
  sizeofGivenBuffer = (sizeofData / DIVISOR_NUMBER);

  // Add modulos of data, if possible
  if (modulusofGivenBuffer > 0)
    sizeofGivenBuffer++;

  // -----

  for (unsigned short index = 0; index < sizeofGivenBuffer; index++) {

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
      givenBuffer[index][subIndex + 2] = (char)IDLE_SINGLE_START;
    else if (index == sizeofGivenBuffer - 1)
      givenBuffer[index][subIndex + 2] = (char)IDLE_MULTI_END;
    else
      givenBuffer[index][subIndex + 2] = (char)IDLE_MULTI_START;

    givenBuffer[index][0] = PROTOCOL_DELIMITERS[0];
    givenBuffer[index][subIndex + 1] = PROTOCOL_DELIMITERS[1];
    givenBuffer[index][subIndex + 3] = PROTOCOL_DELIMITERS[2];
    givenBuffer[index][subIndex + 4] = '\0';
  }

  // If everything goes well, we will arrive here and return true
  return true;
}

// -----

char *generateDelimiterBuffer(char *character, unsigned short sizeofData) {

  if (sizeofData == 0)
    return false;

  // Store count of found on this variable
  char arrayofCharacter[sizeofData + 1];

  for (unsigned short index = 0; index < sizeofData; index++)
    arrayofCharacter[index] = *character;

  // Set last bit as end-of-line bit
  arrayofCharacter[sizeofData] = '\0';

  return arrayofCharacter;
}

unsigned short howManyCharacter(char *character, unsigned short sizeofData, char data[]) {

  if (sizeofData == 0 || data == NULL)
    return 0;

  // Store count of found on this variable
  char countofCharacter = 0;

  for (unsigned short index = 0; index < sizeofData; index++)
    if (*character == data[index])
      countofCharacter++;

  return countofCharacter;
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

