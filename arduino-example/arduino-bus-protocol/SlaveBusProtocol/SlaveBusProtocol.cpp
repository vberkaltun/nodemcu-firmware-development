/*
 * SlaveBusProtocol.cpp
 * SLAVE BUS PROTOCOL - 09.04.2018
 * 
 * =============================================================================
 *
 * The MIT License (MIT)
 * 
 * Copyright (c) 2018 Berk Altun - vberkaltun.com
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * =============================================================================
 */

#include "SlaveBusProtocol.h"

void Slave::clearGivenList() {

    // Clear recent stored data, We will fill again
    givenList.queueID = 0;
    givenList.crcChecksum = 0;
    givenList.returnRequest = false;
    givenList.functionID = NULL;
    givenList.paramSize = 0;
    givenList.paramList = NULL;
}

bool Slave::checkSeparator(char _map) {
    switch (_map) {

        case Slave::separatorData::file:
        case Slave::separatorData::group:
        case Slave::separatorData::record:
        case Slave::separatorData::unit:
            break;

        default:
            return false;
    }

    return true;
}

bool Slave::checkMap(Slave::mapData _selector, char _data[]) {

    // Store the size of map data at the here
    uint16_t sizeofSeparator = strlen(_data);

    // -----

    switch (_selector) {

        case Slave::mapData::queue:

            return true;
            break;

        case Slave::mapData::crc:

            return true;
            break;

        case Slave::mapData::request:

            return true;
            break;

        case Slave::mapData::function:

            return true;
            break;

        case Slave::mapData::param:

            return true;
            break;

        default:
            return false;
    }

    return true;
}

bool Slave::checkSeparatorList(char _data[]) {

    // Store the size of separator data at the here
    uint16_t sizeofSeparator = strlen(_data);

    // -----

    // Ok, Lets start to check data with separator list
    uint16_t checkedSeparator = 0;

    // Declare two iterators for serializing data
    uint16_t previousIndex = 0;
    uint16_t nextIndex = 0;

    for (uint16_t index = 0; index < sizeofSeparator; index++) {

        if (checkSeparator(_data[index])) {

            // Check index on separator list size
            if (checkedSeparator >= DEFAULT_SEPARATORLIST_SIZE)
                return false;

            // Check value on given index in separator list
            if (Slave::separatorList[checkedSeparator++] != _data[index])
                return false;

            // Update index
            previousIndex = nextIndex;
            nextIndex = index;

            // Check whether two separators on back-to-back
            if (nextIndex - previousIndex == 1)
                return false;
        }
    }

    return true;
}

bool Slave::checkMapList(char _data[]) {

    // Store the size of map data at the here
    uint16_t sizeofMap = strlen(_data);

    // -----

    // Copy input string to new string, cause we will change something
    char copiedMap[sizeofMap];
    strcpy(copiedMap, _data);

    // We will use these at changing separator characters
    const char splitCharacter[2] = "\n";

    // Now, Start the search and change what you find with split char
    for (uint16_t index = 0; index < sizeofMap; index++)
        if (checkSeparator(copiedMap[index]))
            copiedMap[index] = splitCharacter[0];

    // -----

    // Store map list index at the here
    uint16_t checkedMap = 0;

    // Store splitting data at the here
    char *mapListPointer = strtok(copiedMap, splitCharacter);

    while (mapListPointer != NULL) {

        // Check index on separator list size
        if (checkedMap >= DEFAULT_MAPLIST_SIZE)
            return false;

        // Only this check is enough, there is no need for further check
        if (!checkMap(Slave::mapList[checkedMap++], mapListPointer))
            return false;

        mapListPointer = strtok(NULL, splitCharacter);
    }

    return true;
}

bool Slave::checkConfigList(char _data[]) {

    // Store the size of config data at the here
    uint16_t sizeofConfig = strlen(_data);

    // -----

    // Check function ID, type is alphanumeric
    for (uint16_t index = 0; index < sizeofConfig; index++)
        if (isalnum(_data[index]) == 0)
            return false;

    return true;
}

bool Slave::checkFunctionList(char _data[]) {

    // Store the size of function data at the here
    uint16_t sizeofFunction = strlen(_data);

    // -----

    if (_data == NULL)
        return false;

    if (sizeofFunction < DEFAULT_FUNCTIONID_SIZE_MIN)
        return false;

    if (sizeofFunction > DEFAULT_FUNCTIONID_SIZE_MAX)
        return false;

    // Check function ID, type is alphanumeric
    for (uint16_t index = 0; index < sizeofFunction; index++)
        if (isalnum(_data[index]) == 0)
            return false;

    // Check is it added before with same ID
    for (uint16_t index = 0; index < DEFAULT_FUNCTIONLIST_SIZE; index++)
        if (strcmp(functionList[index].functionID, _data) == 0)
            return false;

    // ----

    if (configList.functionListCount >= DEFAULT_FUNCTIONLIST_SIZE)
        return false;

    // Yeap, We finally arrived the finish code, now we can malloc and realloc pointer
    if (functionList == NULL)
        functionList = (functionData *) malloc(sizeof (functionData)*(++configList.functionListCount));
    else
        functionList = (functionData *) realloc(functionList, sizeof (functionData)*(++configList.functionListCount));

    return true;
}

bool Slave::decodeData(char _data[]) {

    if (DEFAULT_SEPARATORLIST_SIZE + DEFAULT_MAPLIST_SIZE < DEFAULT_PATTERN_SIZE_MIN)
        return false;

    if (DEFAULT_SEPARATORLIST_SIZE + DEFAULT_MAPLIST_SIZE > DEFAULT_PATTERN_SIZE_MAX)
        return false;

    // This makes absolute compare, whatever abs of variance must be 0 or 1
    if (abs(DEFAULT_SEPARATORLIST_SIZE - DEFAULT_MAPLIST_SIZE) > 1)
        return false;

    // -----

    if (!checkSeparatorList(_data))
        return false;

    if (!checkMapList(_data))

        return false;

    return true;
}

bool Slave::encodeData() {

    return true;
}

// -----

/**
 * Constructors 
 * 
 * @param -
 * @return -
 */
Slave::Slave() {
}

// -----

bool Slave::setFunction(int8_t(*pointer)(uint16_t, char[]), char _functionID[]) {

    bool setFunctionFlag = false;

    if (!configList.setFlag && checkFunctionList(_functionID)) {

        functionList[configList.functionListCount - 1].type = Slave::functionData::is_int8_t;
        functionList[configList.functionListCount - 1].pointer.int8_tValue = pointer;
        functionList[configList.functionListCount - 1].functionID = _functionID;

        setFunctionFlag = true;
    }

    return setFunctionFlag;
}

bool Slave::setFunction(int16_t(*pointer)(uint16_t, char[]), char _functionID[]) {

    bool setFunctionFlag = false;

    if (!configList.setFlag && checkFunctionList(_functionID)) {

        functionList[configList.functionListCount - 1].type = Slave::functionData::is_int16_t;
        functionList[configList.functionListCount - 1].pointer.int16_tValue = pointer;
        functionList[configList.functionListCount - 1].functionID = _functionID;

        setFunctionFlag = true;
    }

    return setFunctionFlag;
}

bool Slave::setFunction(int32_t(*pointer)(uint16_t, char[]), char _functionID[]) {

    bool setFunctionFlag = false;

    if (!configList.setFlag && checkFunctionList(_functionID)) {

        functionList[configList.functionListCount - 1].type = Slave::functionData::is_int32_t;
        functionList[configList.functionListCount - 1].pointer.int32_tValue = pointer;
        functionList[configList.functionListCount - 1].functionID = _functionID;

        setFunctionFlag = true;
    }

    return setFunctionFlag;
}

bool Slave::setFunction(uint8_t(*pointer)(uint16_t, char[]), char _functionID[]) {

    bool setFunctionFlag = false;

    if (!configList.setFlag && checkFunctionList(_functionID)) {

        functionList[configList.functionListCount - 1].type = Slave::functionData::is_uint8_t;
        functionList[configList.functionListCount - 1].pointer.uint8_tValue = pointer;
        functionList[configList.functionListCount - 1].functionID = _functionID;

        setFunctionFlag = true;
    }

    return setFunctionFlag;
}

bool Slave::setFunction(uint16_t(*pointer)(uint16_t, char[]), char _functionID[]) {

    bool setFunctionFlag = false;

    if (!configList.setFlag && checkFunctionList(_functionID)) {

        functionList[configList.functionListCount - 1].type = Slave::functionData::is_uint16_t;
        functionList[configList.functionListCount - 1].pointer.uint16_tValue = pointer;
        functionList[configList.functionListCount - 1].functionID = _functionID;

        setFunctionFlag = true;
    }

    return setFunctionFlag;
}

bool Slave::setFunction(uint32_t(*pointer)(uint16_t, char[]), char _functionID[]) {

    bool setFunctionFlag = false;

    if (!configList.setFlag && checkFunctionList(_functionID)) {

        functionList[configList.functionListCount - 1].type = Slave::functionData::is_uint32_t;
        functionList[configList.functionListCount - 1].pointer.uint32_tValue = pointer;
        functionList[configList.functionListCount - 1].functionID = _functionID;

        setFunctionFlag = true;
    }

    return setFunctionFlag;
}

bool Slave::setFunction(char* (*pointer)(uint16_t, char[]), char _functionID[]) {

    bool setFunctionFlag = false;

    if (!configList.setFlag && checkFunctionList(_functionID)) {

        functionList[configList.functionListCount - 1].type = Slave::functionData::is_char;
        functionList[configList.functionListCount - 1].pointer.charValue = pointer;
        functionList[configList.functionListCount - 1].functionID = _functionID;

        setFunctionFlag = true;
    }

    return setFunctionFlag;
}

bool Slave::setFunction(bool (*pointer)(uint16_t, char[]), char _functionID[]) {

    bool setFunctionFlag = false;

    if (!configList.setFlag && checkFunctionList(_functionID)) {

        functionList[configList.functionListCount - 1].type = Slave::functionData::is_bool;
        functionList[configList.functionListCount - 1].pointer.boolValue = pointer;
        functionList[configList.functionListCount - 1].functionID = _functionID;

        setFunctionFlag = true;
    }

    return setFunctionFlag;
}

bool Slave::setFunction(float (*pointer)(uint16_t, char[]), char _functionID[]) {

    bool setFunctionFlag = false;

    if (!configList.setFlag && checkFunctionList(_functionID)) {

        functionList[configList.functionListCount - 1].type = Slave::functionData::is_float;
        functionList[configList.functionListCount - 1].pointer.floatValue = pointer;
        functionList[configList.functionListCount - 1].functionID = _functionID;

        setFunctionFlag = true;
    }

    return setFunctionFlag;
}

bool Slave::setFunction(void (*pointer)(uint16_t, char[]), char _functionID[]) {

    bool setFunctionFlag = false;

    if (!configList.setFlag && checkFunctionList(_functionID)) {

        functionList[configList.functionListCount - 1].type = Slave::functionData::is_void;
        functionList[configList.functionListCount - 1].pointer.voidValue = pointer;
        functionList[configList.functionListCount - 1].functionID = _functionID;

        setFunctionFlag = true;
    }

    return setFunctionFlag;
}

// -----

bool Slave::setConfig(uint8_t _deviceAddress, char _deviceBrand[], char _deviceID[], char _deviceVersion[]) {

    bool setConfigMapFlag = false;

    if (!configList.setFlag) {

        configList.deviceAddress = _deviceAddress;
        configList.deviceBrand = _deviceBrand;
        configList.deviceID = _deviceID;
        configList.deviceVersion = _deviceVersion;
        configList.setFlag = true;

        // Set config flag as true, after that initialize functions will not work
        setConfigMapFlag = true;
    }

    return setConfigMapFlag;
}

// -----

void Slave::pullData(char _data[]) {

    if (!decodeData(_data) && onUnknown != NULL)
        onUnknown(_data);
}

void Slave::pushData() {
}

// -----

void Slave::onUnknownData(void (*pointer)(char[])) {

    onUnknown = pointer;
}

// -----

/**
 * Preinstantiate Objects
 * 
 * @param -
 * @return -
 */
Slave BusProtocol = Slave();