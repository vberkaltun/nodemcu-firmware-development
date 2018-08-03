#include <Wire.h>

#include <Serializer.h>
#include <MasterScanner.h>
#include <TimerQueue.h>

#include <QueueList.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>

#include <pins_arduino.h>
#include <user_interface.h>

// IMPORTANT NOTICE: These all constant is related with your
// MQTT server and WiFi protocol. In additional, at the here, we are
// Using cloudMQTT server for communication
#define MQTT_USER "XXXXXXXX"
#define MQTT_PASSWORD "XXXXXXXX"
#define MQTT_SERVER "XXXXXXXX"
#define MQTT_PORT 00000
#define WIFI_SSID "XXXXXXXX"
#define WIFI_PASSWORD "XXXXXXXX"

// IMPORTANT NOTICE: These all constant is depending on your protocol
// As you can see, this protocol delimiter was declared in this scope
// That's mean, all function will use this delimiter constant on
// Communication between two or more devices
#define DEVICE_BRAND "intelliPWR Incorporated"
#define DEVICE_MODEL "MasterCore.XX"
#define DEVICE_VERSION "VER 1.0.0"

// // IMPORTANT NOTICE: We must redeclare our bus range because of
// Subslave of slave device. At the here, we do not need to scan all
// These device on the bus. In any case, slave device will scan their
// Own slave device on the bus
#define I2C_BUS_SDA 5
#define I2C_BUS_SCL 4
#define I2C_START_ADDRESS 0x20
#define I2C_STOP_ADDRESS 0x65

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
#define MINIMIZED_BUFFER_SIZE 16

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
enum notifyData {Online, Offline, Confirmed, Unconfirmed};

struct vendorData {
  char Brand[BUFFER_SIZE];
  char Model[BUFFER_SIZE];
  char Version[BUFFER_SIZE];
};

struct functionData {
  char Name[BUFFER_SIZE];
  bool Request = false;
  unsigned short Listen = false;
};

struct deviceData {
  struct vendorData vendorList;
  QueueList<functionData> functionList;
  enum handshakeData handshake = Unknown;
  char address = NULL;
};

QueueList<deviceData> deviceList;
enum communicationData communicationFlag = Idle;
enum notifyData notifyFlag = Offline;

// -----

// Do not change default value of this variable
char receivedBuffer[BUFFER_SIZE * BUFFER_SIZE * MINIMIZED_BUFFER_SIZE];
unsigned short sizeofReceivedBuffer = 0;

// Do not change default value of this variable
char givenBuffer[MINIMIZED_BUFFER_SIZE][BUFFER_SIZE];
unsigned short sizeofGivenBuffer = 0;
unsigned short indexofGivenBuffer = 0;

char sizeofFunctionList = 2;
char *functionList[] = {"getVendors",
                        "getFunctionList"
                       };

// -----

// MQTT client objects, required for MQTT
void callBack(char* topic, byte* payload, unsigned int length);
WiFiClient wifiClient;
PubSubClient mqttClient(MQTT_SERVER, MQTT_PORT, callBack, wifiClient);

// TEMPORARILY, will be delete on release
#define WIRE_BEGIN 0x01

#define BLINK_FREQUENCY 1000
#define BLINK_RANGE 200
#define BLINK_R D6
#define BLINK_GB D7
#define SSR D5

void setup() {

  // Set PWM frequency 500, default is 1000
  analogWriteFreq(BLINK_FREQUENCY);

  // Set range 0 ~ 100, default is 0 ~ 1023
  analogWriteRange(BLINK_RANGE);

  // Initialize RGB port and device listen port
  pinMode(BLINK_R, OUTPUT);
  pinMode(BLINK_GB, OUTPUT);
  pinMode(SSR, OUTPUT);

  // Reset pins
  analogWrite(BLINK_R, BLINK_RANGE);
  analogWrite(BLINK_GB, BLINK_RANGE);
  digitalWrite(SSR, HIGH);

  // -----

  // Initialize communication on wire and serial protocol
  Serial.begin(9600);

  // Initialize communication on Wire protocol
  Wire.begin(I2C_BUS_SDA, I2C_BUS_SCL);
  Wire.begin(WIRE_BEGIN);

  // A name when discovering it as a port in ARDUINO IDE
  ArduinoOTA.setHostname(DEVICE_MODEL);
  ArduinoOTA.begin();

  // Start wifi subsystem
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  connectWiFi();

  // -----

  // IMPORTANT NOTICE: Due to our I2C scanner lib, When a new device
  // Connected or disconnected to master, our I2C scanner lib decides
  // Which one is to be triggered
  MasterScanner.setRange(I2C_START_ADDRESS, I2C_STOP_ADDRESS);
  MasterScanner.onConnectedSlaves(connectedSlaves);
  MasterScanner.onDisconnectedSlaves(disconnectedSlaves);

  // Attach functions to lib and after run main lib
  TimerQueue.attach(listenSlave, (unsigned long)1000);
  TimerQueue.attach(listenFunction, (unsigned long)10);
  TimerQueue.attach(listenWiFi, (unsigned long)20);
  TimerQueue.start();
}

void loop() {

  // IMPORTANT NOTICE: As you can see, we do not use delay function
  // In all lib. Delay function is a non-blocking function in Arduino
  // Core. So solving this, we are using MILLIS()
  TimerQueue.loop();
}

void listenSlave() {
  MasterScanner.scanSlaves();
}

void listenFunction() {

  // IMPORTANT NOTICE: Device registering is more priority than others
  // Step, When new device(s) were connected to master device, firstly
  // Register these device(s) to system, after continue what you do
  for (unsigned short index = 0; index < deviceList.size(); index++) {

    // If current index is empty, go next
    if (deviceList[index].functionList.size() == 0)
      continue;

    // If handshake is not ok, that's mean probably function is also not ok
    if (deviceList[index].handshake != Ready)
      continue;

    // Listen functions on connected device(s)
    for (unsigned short subindex = 0; subindex < deviceList[index].functionList.size(); subindex++) {

      if (!deviceList[index].functionList[subindex].Listen)
        continue;

      // Store func name at here, we can not use it directly
      char internalData1[BUFFER_SIZE];
      sprintf(internalData1, "%s", deviceList[index].functionList[subindex].Name);

      char *encodeDelimiter1 = generateDelimiterBuffer(DATA_DELIMITER, 1);
      char *resultBuffer1[] = {internalData1, "NULL"};
      char *result1 = Serialization.encode(1, encodeDelimiter1, 2, resultBuffer1);

      // Write internal function list to connected device, one-by-one
      writeData(deviceList[index].address, result1);

      // We will do this till decoding will return false
      while (true) {

        // More info was given at inside of this function
        // Actually, that's not worst case
        if (!readData(deviceList[index].address))
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

      // -----

      // Decode last given data from slave, after we will publish it
      char *subDelimiter = generateDelimiterBuffer(DATA_DELIMITER, 1);
      char **subBuffer = Serialization.decode(1, subDelimiter, sizeofReceivedBuffer, receivedBuffer);

      // Null operator check
      if (subBuffer == NULL)
        break;

      // -----

      // Store func name at here, we can not use it directly
      char internalData2[BUFFER_SIZE];
      sprintf(internalData2, "0x%2x", deviceList[index].address);

      // Looking good, inline if-loop
      char *encodeDelimiter2 = generateDelimiterBuffer("/", 2);
      char *resultBuffer2[] = {internalData2, internalData1};
      char *result2 = Serialization.encode(2, encodeDelimiter2, 2, resultBuffer2);

      // Publish last received buffer to MQTT broker
      mqttClient.publish(result2, subBuffer[1]);
    }
  }
}

void listenWiFi() {

  // Reconnect if connection is lost
  if (!mqttClient.connected() && WiFi.status() == 3)
    connectWiFi();

  // Maintain MQTT connection
  mqttClient.loop();

  // Allow Wi-Fi functions to run
  ArduinoOTA.handle();
}

// -----

void connectedSlaves(uint8_t data[], byte sizeofData) {

  // IMPORTANT NOTICE: Before the calling internal functions,
  // Last stored data must be removed on memory. Otherwise, we can not sent
  // Last stored data to master device. And additional, data removing will refresh
  // the size of data in memory. This is most important thing ...
  for (char index = 0; index < sizeofGivenBuffer; index++)
    for (char subindex = 0; subindex < BUFFER_SIZE; subindex++)
      givenBuffer[index][subindex] = '\0';

  sizeofGivenBuffer = 0;
  indexofGivenBuffer = 0;

  // Free up out-of-date buffer data
  receivedBuffer[0] = '\0';
  sizeofReceivedBuffer = 0;

  // -----

  Serial.print("Done! ");
  Serial.print(sizeofData, DEC);
  Serial.println(" slave device(s) connected to I2C bus. ID(s): ");

  for (unsigned short index = 0; index < sizeofData; index++) {
    Serial.print("\t ID: 0x");
    Serial.println(data[index], HEX);

  }
  Serial.println("\t ---");

  // Notify end user, status is device online
  notifyBlink(0, Online);

  // -----

  // IMPORTANT NOTICE: Device registering is more priority than others
  // Step, When new device(s) were connected to master device, firstly
  // Register these device(s) to system, after continue what you do
  for (unsigned short index = 0; index < sizeofData; index++) {

    // Register last added device to system
    for (unsigned short subindex = 0; subindex < sizeofFunctionList; subindex++) {

      char *encodeDelimiter = generateDelimiterBuffer(DATA_DELIMITER, 1);
      char *resultBuffer[] = {functionList[subindex], "NULL"};
      char *result = Serialization.encode(1, encodeDelimiter, 2, resultBuffer);

      // Write internal function list to connected device, one-by-one
      writeData(data[index], result);

      // We will do this till decoding will return false
      while (true) {

        // More info was given at inside of this function
        // Actually, that's not worst case
        if (!readData(data[index]))
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
      if (!fillConfigurationData(subindex, data[index]))
        break;

      // -----

      // Subscribe all function of current slave to MQTT broker
      subscribeTopic(data[index], true);
    }
  }
}

void disconnectedSlaves(uint8_t data[], byte sizeofData) {

  // IMPORTANT NOTICE: Before the calling internal functions,
  // Last stored data must be removed on memory. Otherwise, we can not sent
  // Last stored data to master device. And additional, data removing will refresh
  // the size of data in memory. This is most important thing ...
  for (char index = 0; index < sizeofGivenBuffer; index++)
    for (char subindex = 0; subindex < BUFFER_SIZE; subindex++)
      givenBuffer[index][subindex] = '\0';

  sizeofGivenBuffer = 0;
  indexofGivenBuffer = 0;

  // Free up out-of-date buffer data
  receivedBuffer[0] = '\0';
  sizeofReceivedBuffer = 0;

  // -----

  Serial.print("Done! ");
  Serial.print(sizeofData, DEC);
  Serial.println(" slave device(s) disconnected from I2C bus.");

  for (unsigned short index = 0; index < sizeofData; index++) {
    Serial.print("\t ID: 0x");
    Serial.println(data[index], HEX);

    // Unsubscribe all function of current slave to MQTT broker
    subscribeTopic(data[index], false);
  }
  Serial.println("\t ---");

  // Notify end user, status is device online
  notifyBlink(0, Offline);

  // -----

  // IMPORTANT NOTICE: At the here, firstly, we will make a search about
  // Disconnected device. When we find it, We will delete index of this
  // Device, and after we will add all temporarily popped device again
  // If disconnected count is equal to size of device data, delete all
  if (sizeofData >= deviceList.size())
    deviceList.clear();
  else {

    for (unsigned short index = 0; index < sizeofData; index++) {

      // Just to be on the safe side, check that queue is empty or not
      if (deviceList.size() == 0)
        break;

      if (data[index] == deviceList[0].address) {
        deviceList.popFront();
        continue;
      }

      if (data[index] == deviceList[deviceList.size() - 1].address) {
        deviceList.popBack();
        continue;
      }

      for (unsigned short subindex = 1; subindex < deviceList.size() - 1; subindex++) {
        if (deviceList[subindex].address == data[index]) {

          // IMPORTANT NOTICE: In this func, we are deleting information about
          // Disconnected device. For now, We can not delete it directly. Because
          // Of this, for solving this, firstly we will clone first item to there
          // Secondly we will delete this first item from queue
          copyConfigurationData(0, subindex);
          deviceList.popFront();

          // We found, stop the current session
          break;
        }
      }
    }
  }
}

// -----

void unknownEvent(unsigned short sizeofData, char data[]) {

  // IMPORTANT NOTICE: Before the calling internal functions,
  // Last stored data must be removed on memory. Otherwise, we can not sent
  // Last stored data to master device. And additional, data removing will refresh
  // the size of data in memory. This is most important thing ...
  for (char index = 0; index < sizeofGivenBuffer; index++)
    for (char subindex = 0; subindex < BUFFER_SIZE; subindex++)
      givenBuffer[index][subindex] = '\0';

  sizeofGivenBuffer = 0;
  indexofGivenBuffer = 0;

  // Free up out-of-date buffer data
  receivedBuffer[0] = '\0';
  sizeofReceivedBuffer = 0;

  // -----

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

bool subscribeTopic(char address, bool type) {

  // Make sure we are connected to WIFI before attemping to reconnect to MQTT
  if (WiFi.status() != WL_CONNECTED)
    return false;

  // Make sure we are connected to the MQTT server
  if (!mqttClient.connected())
    return false;

  // -----

  // When we found given device in device list, generate MQTT vendor(s)
  for (unsigned short index = 0; index < deviceList.size(); index++) {
    if (address == deviceList[index].address) {

      // Store func name at here, we can not use it directly
      char internalData[BUFFER_SIZE];
      sprintf(internalData, "0x%2x", address);

      // Looking good, inline if-loop
      char *encodeDelimiter1 = generateDelimiterBuffer("/", 2);
      char *resultBuffer1[] = {internalData, "isConnected"};
      char *result1 = Serialization.encode(2, encodeDelimiter1, 2, resultBuffer1);
      type ? mqttClient.subscribe(result1) : mqttClient.unsubscribe(result1);
      mqttClient.publish(result1, type ? "1" : "0");

      char *encodeDelimiter2 = generateDelimiterBuffer("/", 2);
      char *resultBuffer2[] = {internalData, "brand"};
      char *result2 = Serialization.encode(2, encodeDelimiter2, 2, resultBuffer2);
      type ? mqttClient.publish(result2, deviceList[index].vendorList.Brand) : NULL;

      char *encodeDelimiter3 = generateDelimiterBuffer("/", 2);
      char *resultBuffer3[] = {internalData, "model"};
      char *result3 = Serialization.encode(2, encodeDelimiter3, 2, resultBuffer3);
      type ? mqttClient.publish(result3,  deviceList[index].vendorList.Model) : NULL;

      char *encodeDelimiter4 = generateDelimiterBuffer("/", 2);
      char *resultBuffer4[] = {internalData, "version"};
      char *result4 = Serialization.encode(2, encodeDelimiter4, 2, resultBuffer4);
      type ? mqttClient.publish(result4, deviceList[index].vendorList.Version) : NULL;

      // -----

      for (unsigned short subindex = 0; subindex < deviceList[index].functionList.size(); subindex++) {

        if (!deviceList[index].functionList[subindex].Request)
          continue;

        // Store func name at here, we can not use it directly
        char subInternalData0[BUFFER_SIZE * MINIMIZED_BUFFER_SIZE];
        sprintf(subInternalData0, "0x%2x", deviceList[index].address);

        // Store address at here, we can not use it directly
        char subInternalData1[BUFFER_SIZE * MINIMIZED_BUFFER_SIZE];
        sprintf(subInternalData1, "%s", deviceList[index].functionList[subindex].Name);

        char *subEncodeDelimiter = generateDelimiterBuffer("/", 2);
        char *subResultBuffer[] = {subInternalData0, subInternalData1};
        char *subResult = Serialization.encode(2, subEncodeDelimiter, 2, subResultBuffer);

        // Looking good, inline if-loop
        type ? mqttClient.subscribe(subResult) : mqttClient.unsubscribe(subResult);
      }

      // Do not need to search all data, we are OK now
      break;
    }
  }
}

void callBack(char* topic, byte * payload, unsigned int length) {

  // Convert topic to string to make it easier to work with
  String stringofTopic = topic;
  unsigned short sizeofTopic = stringofTopic.length();

  // Print out some debugging info
  Serial.print("Done! Callback updated on <");
  Serial.print(stringofTopic);
  Serial.print(">[");
  Serial.print(length);
  Serial.println("].");

  // Decode payload to char array
  char payloadData[length + 1];
  for (unsigned short subindex = 0; subindex < sizeofTopic; subindex++)
    payloadData[subindex] = (char)payload[subindex];
  payloadData[length] = '\0';

  // -----

  // Generate a delimiter data and use in with decoding function
  char *decodeDelimiter = generateDelimiterBuffer("/", 2);
  char **decodeBuffer = Serialization.decode(2, decodeDelimiter, sizeofTopic, topic);

  // Null operator check
  if (decodeBuffer != NULL) {

    char internalData0[BUFFER_SIZE];
    sprintf(internalData0, "%s", decodeBuffer[0]);
    unsigned short sizeofInternalData0 = strlen(internalData0);

    char internalData1[BUFFER_SIZE];
    sprintf(internalData1, "%s", decodeBuffer[1]);
    unsigned short sizeofInternalData1 = strlen(internalData1);

    // -----

    // Declare a flag, related with device status
    bool deviceModelFlag = true;

    // Compare internal data(s) with device model, higher priority
    for (unsigned short index = 0; index < strlen(DEVICE_MODEL); index++) {
      if (internalData0[index] != DEVICE_MODEL[index]) {
        deviceModelFlag = false;
        break;
      }
    }

    // IMPORTANT NOTICE: If we can arrive there, that's mean the payload
    // Data is consisted of status data. Otherwise, we can say the payload
    // Data is not consisted of status data. Worst case, Go other state
    // Of if, that's mean also it is a function data(s)
    if (deviceModelFlag && isNumeric(1, payloadData))
      digitalWrite(SSR, (bool)(payload[0] - '0'));

    // -----

    // Compare internal data(s) with registered function list
    for (unsigned short index = 0; index < deviceList.size(); index++) {

      // Store func name at the here, we can not use it directly
      char internalData2[BUFFER_SIZE];
      sprintf(internalData2, "0x%2x", deviceList[index].address);

      // Check thar is it same or not
      for (unsigned short subindex = 0; subindex < sizeofInternalData0; subindex++)
        if (internalData0[subindex] != internalData2[subindex])
          break;

      // -----

      char *encodeDelimiter = generateDelimiterBuffer(DATA_DELIMITER, 1);
      char *resultBuffer[] = {internalData1, payloadData};
      char *result = Serialization.encode(1, encodeDelimiter, 2, resultBuffer);

      // Write internal data list to connected device, one-by-one
      writeData(deviceList[index].address, result);
    }
  }
}

void connectWiFi() {

  // Attempt to connect to the wifi if connection is lost
  if (WiFi.status() != WL_CONNECTED) {

    // Notify user
    Serial.print("Connecting to <");
    Serial.print(WIFI_SSID);
    Serial.print("> ...");

    // Loop while we wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    // Notify user
    Serial.println("");
    Serial.print("Done! WiFi connection is OK. IP address was given as <");
    Serial.print(WiFi.localIP());
    Serial.println(">.");
  }

  // If we are not connected to server, try to connect server
  while (!mqttClient.connected()) {
    if (mqttClient.connect((char*) DEVICE_MODEL, MQTT_USER, MQTT_PASSWORD)) {

      // IMPORTANT NOTICE: First of all, we need to subscribe main device
      // We call it like XXXX/status and this broker is related with SSR
      // State of device. We will listen something about this and execute it
      char *encodeDelimiter = generateDelimiterBuffer("/", 2);
      char *resultBuffer[] = {DEVICE_MODEL, "status"};
      char *result = Serialization.encode(2, encodeDelimiter, 2, resultBuffer);
      mqttClient.subscribe(result);
    }
  }
}

void notifyBlink(unsigned short port, enum notifyData status) {

  switch (status) {
    case Online:
      for (int index = 0; index < 3; index++) {
        for (int subindex = BLINK_RANGE; subindex > 0; subindex--) {
          analogWrite(BLINK_R, subindex);
          analogWrite(BLINK_GB, subindex);
          delay(1);
        }
        delay(400);
        for (int subindex = 0; subindex < BLINK_RANGE; subindex++) {
          analogWrite(BLINK_R, subindex);
          analogWrite(BLINK_GB, subindex);
          delay(2);
        }
        delay(800);
      }
      for (int index = BLINK_RANGE; index > 0; index--) {
        analogWrite(BLINK_R, index);
        analogWrite(BLINK_GB, index);
        delay(1);
      }
      // Update last stored notify flag
      notifyFlag = Online;
      break;

    case Offline:
      switch (notifyFlag) {
        case Online:
          for (int index = 0; index < BLINK_RANGE; index++) {
            analogWrite(BLINK_R, index);
            analogWrite(BLINK_GB, index);
            delay(1);
          }
          break;

        case Confirmed:
          for (int index = 0; index < BLINK_RANGE; index++) {
            analogWrite(BLINK_GB, index);
            delay(1);
          }
          break;

        case Unconfirmed:
          for (int index = 0; index < BLINK_RANGE; index++) {
            analogWrite(BLINK_R, index);
            delay(1);
          }
          break;

        default:
          analogWrite(BLINK_R, BLINK_RANGE);
          analogWrite(BLINK_GB, BLINK_RANGE);
          break;
      }
      // Update last stored notify flag
      notifyFlag = Offline;
      break;

    case Confirmed:
      for (int index = 0; index < BLINK_RANGE; index++) {
        analogWrite(BLINK_R, index);
        delay(1);
      }
      // Update last stored notify flag
      notifyFlag = Confirmed;
      break;

    case Unconfirmed:
      for (int index = BLINK_RANGE; index > 0; index--) {
        analogWrite(BLINK_GB, index);
        delay(1);
      }
      // Update last stored notify flag
      notifyFlag = Unconfirmed;
      break;

    default:
      analogWrite(BLINK_R, BLINK_RANGE);
      analogWrite(BLINK_GB, BLINK_RANGE);
      // Update last stored notify flag
      notifyFlag = Offline;
      break;
  }
}

// -----

bool writeData(char address, char *data) {

  // Free up out-of-date buffer data, top level clearing
  receivedBuffer[0] = '\0';
  sizeofReceivedBuffer = 0;

  // Encode given data for bus communication
  encodeData(strlen(data), data);

  // Send all remainder data to newly registered slave
  while (indexofGivenBuffer < sizeofGivenBuffer) {
    Wire.beginTransmission(address);
    Wire.write(givenBuffer[indexofGivenBuffer++]);
    Wire.endTransmission();

    // Maybe not need, right?
    delay(15);
  }

  // IMPORTANT NOTICE: Due to decoding of slave device, we need to Wait
  // A little bit. Otherwise, Master device will request data From slave
  // Device too early and slave cannot send it. Additional, when more
  // Devices are connected, we need to downgrade delay time. Already,
  // It will take the same time during roaming
  delay(100);

  // If everything goes well, we will arrive here and return true
  return true;
}

bool readData(char address) {

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

  // Maybe not need, right?
  delay(15);

  // Decode last given data
  if (!decodeData(indexofNewReceivedBuffer, newReceivedBuffer))
    return false;

  // IMPORTANT NOTICE: Actually When we arrived this point, we arrived
  // Worst case point even though It was TRUE. If you came there, program will
  // Run till communication flag will be END or IDLE type. Otherwise, this
  // Point is related with CONTINUE status
  return true;
}

// -----

void copyConfigurationData(char fromAddress, char toAddress) {

  // Worst case, We do not have another solution
  for (unsigned short index = 0; index < BUFFER_SIZE; index++) {
    if (deviceList[fromAddress].vendorList.Brand[index] == NULL) {
      deviceList[toAddress].vendorList.Brand[index] = '\0';
      break;
    }
    deviceList[toAddress].vendorList.Brand[index] = deviceList[fromAddress].vendorList.Brand[index];
  }

  for (unsigned short index = 0; index < BUFFER_SIZE; index++) {
    if (deviceList[fromAddress].vendorList.Model[index] == NULL) {
      deviceList[toAddress].vendorList.Model[index] = '\0';
      break;
    }
    deviceList[toAddress].vendorList.Model[index] = deviceList[fromAddress].vendorList.Model[index];
  }

  for (unsigned short index = 0; index < BUFFER_SIZE; index++) {
    if (deviceList[fromAddress].vendorList.Version[index] == NULL) {
      deviceList[toAddress].vendorList.Version[index] = '\0';
      break;
    }
    deviceList[toAddress].vendorList.Version[index] = deviceList[fromAddress].vendorList.Version[index];
  }

  // -----

  // Worst case, We do not have another solution
  for (unsigned short index = 0; index < deviceList[fromAddress].functionList.size(); index++) {

    for (unsigned short subindex = 0; subindex < BUFFER_SIZE; subindex++) {
      if (deviceList[fromAddress].functionList[index].Name[subindex] == NULL) {
        deviceList[toAddress].functionList[index].Name[subindex] = '\0';
        break;
      }
      deviceList[toAddress].functionList[index].Name[subindex] =  deviceList[fromAddress].functionList[index].Name[subindex];
    }

    deviceList[toAddress].functionList[index].Request = deviceList[fromAddress].functionList[index].Request;
    deviceList[toAddress].functionList[index].Listen = deviceList[fromAddress].functionList[index].Listen;
  }

  deviceList[toAddress].handshake = deviceList[fromAddress].handshake;
  deviceList[toAddress].address = deviceList[fromAddress].address;
}

bool fillConfigurationData(unsigned short index, char address) {

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
      if (!fillVendorData(address, countofCharacter, newReceivedBuffer))
        return false;

      // Notify user
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
      if (!fillFunctionData(address, countofCharacter, newReceivedBuffer)) {
        // Notify end user, status is device online
        notifyBlink(0, Unconfirmed);
        return false;
      }

      // Notify end user, status is device online
      notifyBlink(0, Confirmed);

      // Notify user
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
  if (sizeofData != 2) {

    struct deviceData newDeviceData;
    newDeviceData.handshake = Unknown;
    newDeviceData.address = address;

    // Major code for device list
    deviceList.pushFront(newDeviceData);
    unknownEvent(sizeofReceivedBuffer, receivedBuffer);

    return false;
  }

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
      continue;

    char internalData1[BUFFER_SIZE];
    sprintf(internalData1, "%s", data[index + 1]);
    unsigned short sizeofInternalData1 = strlen(internalData1);
    if (!isNumeric(sizeofInternalData1, internalData1))
      continue;

    char internalData2[BUFFER_SIZE];
    sprintf(internalData2, "%s", data[index + 2]);
    unsigned short sizeofInternalData2 = strlen(internalData2);
    if (!isNumeric(sizeofInternalData2, internalData2))
      continue;

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
    newFunctionData.Listen = (bool)atoi(internalData2);

    // If we can arrive there, we can say function data is ok
    deviceList[0].functionList.pushFront(newFunctionData);
  }

  // Set other important externals data to internals list
  deviceList[0].handshake = Ready;
  deviceList[0].address = address;

  // If everything goes well, we will arrive here and return true
  return true;
}

// -----

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
    for (char subindex = 0; subindex < BUFFER_SIZE; subindex++)
      givenBuffer[index][subindex] = '\0';

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
    unsigned short upperBound = 0;

    // Calculate maximum upperbound of iterator
    if (modulusofGivenBuffer != 0 && index == sizeofGivenBuffer - 1)
      upperBound = modulusofGivenBuffer;
    else
      upperBound = DIVISOR_NUMBER;
      
    // Change upper bound if module is available and index is on the last one
    if (index == sizeofGivenBuffer - 1 && modulusofGivenBuffer != 0)
      upperBound = modulusofGivenBuffer;

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
  static char arrayofCharacter[BUFFER_SIZE * MINIMIZED_BUFFER_SIZE];

  // Set first bit as end-of-line biti this is for initialize
  arrayofCharacter[0] = '\0';

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
