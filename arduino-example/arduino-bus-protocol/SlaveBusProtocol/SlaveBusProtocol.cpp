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
    givenList.queue = 0;
    givenList.crc = 0;
    givenList.request = false;
    givenList.paramSize = 0;

    // IMPORTANT NOTICE: seems so confused, right? 
    // When you free up a pointer, you can not use it again anymore 
    // But when you makw reassigning NULL to a pointer after the freeing up, 
    // You can this pointer again very well.
    free(givenList.function);
    givenList.function = NULL;
    free(givenList.paramList);
    givenList.paramList = NULL;
}

bool Slave::checkSeparator(char _map) {

    switch (_map) {

        case separatorData::file:
        case separatorData::group:
        case separatorData::record:
        case separatorData::unit:
            break;

        default:
            return false;
    }

    return true;
}

bool Slave::checkMap(mapData _selector, char _data[]) {

    // IMPORTANT NOTICE: This function is based on your data
    // You can change and develop all of this func depending on your objectives
    // For some step, I did not declare a new DEFINE on the top of "Header"
    // I made it all declaring at the here for to avoid confusion

    // Store the size of map data at the here
    uint16_t sizeofSeparator = strlen(_data);

    // -----

    switch (_selector) {

        case mapData::queue:

            if (sizeofSeparator > 3)
                return false;

            for (uint16_t index = 0; index < sizeofSeparator; index++)
                if (isdigit(_data[0]) == 0)
                    return false;

            // Finally, store it. not need compare
            givenList.queue = atoi(_data);

            if (givenList.queue > 255)
                return false;

            break;

        case mapData::crc:

            if (sizeofSeparator > 5)
                return false;

            for (uint16_t index = 0; index < sizeofSeparator; index++)
                if (isdigit(_data[0]) == 0)
                    return false;

            // Finally, store it. not need compare
            givenList.crc = atoi(_data);

            break;

        case mapData::request:

            if (sizeofSeparator > 1)
                return false;

            if (isdigit(_data[0]) == 0)
                return false;

            // Finally, store it. not need compare
            givenList.request = atoi(_data);

            break;

        case mapData::function:

            if (sizeofSeparator < DEFAULT_FUNCTION_SIZE_MIN)
                return false;

            if (sizeofSeparator > DEFAULT_FUNCTION_SIZE_MAX)
                return false;

            for (uint16_t index = 0; index < sizeofSeparator; index++)
                if (isalnum(_data[index]) == 0)
                    return false;

            // Malloc a word, a list of characters, after carry it
            givenList.function = (char *) malloc(sizeof (char) * sizeofSeparator);
            strcpy(givenList.function, _data);

            break;

        case mapData::param:

            if (sizeofSeparator > 32)
                return false;

            for (uint16_t index = 0; index < sizeofSeparator; index++)
                if (isalnum(_data[index]) == 0)
                    return false;

            // Malloc and realloc a sentence,  a list of words
            if (givenList.paramList == NULL)
                givenList.paramList = (char **) malloc(sizeof (char *) * (++givenList.paramSize));
            else
                givenList.paramList = (char **) realloc(givenList.paramList, sizeof (char *) * (++givenList.paramSize));

            // Malloc and realloc a word, a list of characters, after carry it
            givenList.paramList[givenList.paramSize - 1] = (char *) malloc(sizeof (char) * 32);
            strcpy(givenList.paramList[givenList.paramSize - 1], _data);

            break;

        default:
            return false;
    }

    return true;
}

bool Slave::checkFunction(char _data[]) {

    // Store the size of function data at the here
    uint16_t sizeofFunction = strlen(_data);

    // -----

    if (_data == NULL)
        return false;

    if (sizeofFunction < DEFAULT_FUNCTION_SIZE_MIN)
        return false;

    if (sizeofFunction > DEFAULT_FUNCTION_SIZE_MAX)
        return false;

    // Check function ID, type is alphanumeric
    for (uint16_t index = 0; index < sizeofFunction; index++)
        if (isalnum(_data[index]) == 0)
            return false;

    // Check is it added before with same ID
    for (uint16_t index = 0; index < DEFAULT_FUNCTIONLIST_SIZE; index++)
        if (strcmp(functionList[index].function, _data) == 0)
            return false;

    return true;
}

bool Slave::checkSeparatorList(char _data[]) {

    // Store the size of separator data at the here
    uint16_t sizeofSeparator = strlen(_data);

    // -----

    // Ok, Lets start to check data with separator list
    uint16_t checkedSeparator = 0;

    // Declare an iterators for storing last visited data
    uint16_t previousIndex = 0;

    for (uint16_t currentIndex = 0; currentIndex < sizeofSeparator; currentIndex++) {

        if (checkSeparator(_data[currentIndex])) {

            // Check index on separator list size
            if (checkedSeparator >= DEFAULT_SEPARATORLIST_SIZE)
                return false;

            // Check value on given index in separator list
            if (separatorList[checkedSeparator++] != _data[currentIndex])
                return false;

            // Check whether two separators on back-to-back
            if (currentIndex - previousIndex == 1)
                return false;

            // Update index
            previousIndex = currentIndex;
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
        if (!checkMap(mapList[checkedMap++], mapListPointer))
            return false;

        mapListPointer = strtok(NULL, splitCharacter);
    }

    Serial.print(givenList.queue);
    Serial.print(givenList.crc);
    Serial.print(givenList.request);
    Serial.print(givenList.function);

    for (uint8_t index; index < givenList.paramSize; index++)
        Serial.print(givenList.paramList[index]);

    Serial.println();

    return true;
}

bool Slave::checkFunctionList() {

    if (configList.functionListSize >= DEFAULT_FUNCTIONLIST_SIZE)
        return false;

    // Yeap, We finally arrived the finish code, now we can malloc and realloc pointer
    if (functionList == NULL)
        functionList = (functionData *) malloc(sizeof (functionData)*(++configList.functionListSize));
    else
        functionList = (functionData *) realloc(functionList, sizeof (functionData)*(++configList.functionListSize));

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

bool Slave::setFunction(int8_t(*pointer)(uint16_t, char[]), char _function[]) {

    if (!checkFunction(_function) && !checkFunctionList())
        return false;

    functionList[configList.functionListSize - 1].type = functionData::is_INT8_T;
    functionList[configList.functionListSize - 1].pointer.INT8_T = pointer;
    functionList[configList.functionListSize - 1].function = _function;

    return true;
}

bool Slave::setFunction(int16_t(*pointer)(uint16_t, char[]), char _function[]) {

    if (!checkFunction(_function) && !checkFunctionList())
        return false;
    functionList[configList.functionListSize - 1].type = functionData::is_INT16_T;
    functionList[configList.functionListSize - 1].pointer.INT16_T = pointer;
    functionList[configList.functionListSize - 1].function = _function;

    return true;
}

bool Slave::setFunction(int32_t(*pointer)(uint16_t, char[]), char _function[]) {

    if (!checkFunction(_function) && !checkFunctionList())
        return false;

    functionList[configList.functionListSize - 1].type = functionData::is_INT32_T;
    functionList[configList.functionListSize - 1].pointer.INT32_T = pointer;
    functionList[configList.functionListSize - 1].function = _function;

    return true;
}

bool Slave::setFunction(uint8_t(*pointer)(uint16_t, char[]), char _function[]) {

    if (!checkFunction(_function) && !checkFunctionList())
        return false;

    functionList[configList.functionListSize - 1].type = functionData::is_UINT8_T;
    functionList[configList.functionListSize - 1].pointer.UINT8_T = pointer;
    functionList[configList.functionListSize - 1].function = _function;

    return true;
}

bool Slave::setFunction(uint16_t(*pointer)(uint16_t, char[]), char _function[]) {

    if (!checkFunction(_function) && !checkFunctionList())
        return false;

    functionList[configList.functionListSize - 1].type = functionData::is_UINT16_T;
    functionList[configList.functionListSize - 1].pointer.UINT16_T = pointer;
    functionList[configList.functionListSize - 1].function = _function;

    return true;
}

bool Slave::setFunction(uint32_t(*pointer)(uint16_t, char[]), char _function[]) {

    if (!checkFunction(_function) && !checkFunctionList())
        return false;

    functionList[configList.functionListSize - 1].type = functionData::is_UINT32_T;
    functionList[configList.functionListSize - 1].pointer.UINT32_T = pointer;
    functionList[configList.functionListSize - 1].function = _function;

    return true;
}

bool Slave::setFunction(char* (*pointer)(uint16_t, char[]), char _function[]) {

    if (!checkFunction(_function) && !checkFunctionList())
        return false;

    functionList[configList.functionListSize - 1].type = functionData::is_CHAR;
    functionList[configList.functionListSize - 1].pointer.CHAR = pointer;
    functionList[configList.functionListSize - 1].function = _function;

    return true;
}

bool Slave::setFunction(bool (*pointer)(uint16_t, char[]), char _function[]) {

    if (!checkFunction(_function) && !checkFunctionList())
        return false;

    functionList[configList.functionListSize - 1].type = functionData::is_BOOL;
    functionList[configList.functionListSize - 1].pointer.BOOL = pointer;
    functionList[configList.functionListSize - 1].function = _function;

    return true;
}

bool Slave::setFunction(float (*pointer)(uint16_t, char[]), char _function[]) {

    if (!checkFunction(_function) && !checkFunctionList())
        return false;

    functionList[configList.functionListSize - 1].type = functionData::is_FLOAT;
    functionList[configList.functionListSize - 1].pointer.FLOAT = pointer;
    functionList[configList.functionListSize - 1].function = _function;

    return true;
}

bool Slave::setFunction(void (*pointer)(uint16_t, char[]), char _function[]) {

    if (!checkFunction(_function) && !checkFunctionList())
        return false;

    functionList[configList.functionListSize - 1].type = functionData::is_VOID;
    functionList[configList.functionListSize - 1].pointer.VOID = pointer;
    functionList[configList.functionListSize - 1].function = _function;

    return true;
}

// -----

void Slave::pullData(char _data[]) {

    // First, clear recent given data
    clearGivenList();

    if (!decodeData(_data) && configList.onUnknownPointer != NULL)
        configList.onUnknownPointer(_data);
}

void Slave::pushData() {
}

// -----

void Slave::onUnknownData(void (*pointer)(char[])) {

    configList.onUnknownPointer = pointer;
}

// -----

/**
 * Preinstantiate Objects
 * 
 * @param -
 * @return -
 */
Slave BusProtocol = Slave();