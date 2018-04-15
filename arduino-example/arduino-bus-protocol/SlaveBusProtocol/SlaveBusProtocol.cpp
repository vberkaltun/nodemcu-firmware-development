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
    givenList.paramSize = 0;

    // IMPORTANT NOTICE: seems so confused, right? 
    // When you free up a pointer, you can not use it again anymore 
    // But when you makw reassigning NULL to a pointer after the freeing up, 
    // You can this pointer again very well.

    free(givenList.paramList);
    givenList.paramList = NULL;

    free(givenList.function);
    givenList.function = NULL;
}

// -----

bool Slave::isNumeric(uint8_t _lowerBound, uint8_t _upperBound, char _data[]) {

    // Store the size of map data at the here
    uint16_t sizeofSeparator = strlen(_data);

    // -----

    if (_data == NULL)
        return false;

    for (uint16_t index = 0; index < sizeofSeparator; index++)
        if (isdigit(_data[0]) == 0)
            return false;

    if (atoi(_data) < _lowerBound)
        return false;

    if (atoi(_data) > _upperBound)
        return false;

    return true;
}

bool Slave::isAlphanumeric(uint8_t _lowerBound, uint8_t _upperBound, char _data[]) {

    // Store the size of function data at the here
    uint16_t sizeofFunction = strlen(_data);

    // -----

    if (_data == NULL)
        return false;

    if (sizeofFunction < _lowerBound)
        return false;

    if (sizeofFunction > _upperBound)
        return false;

    // Check function ID, type is alphanumeric
    for (uint16_t index = 0; index < sizeofFunction; index++)
        if (isalnum(_data[index]) == 0)
            return false;

    return true;
}

bool Slave::isSeparator(char _selector) {

    switch (_selector) {

        case separatorData::file:
        case separatorData::group:
            break;

        default:
            return false;
    }

    return true;
}

bool Slave::isMap(mapData _selector, char _data[]) {

    // IMPORTANT NOTICE: This function is based on your data
    // You can change and develop all of this func depending on your objectives
    // For some step, I did not declare a new DEFINE on the top of "Header"
    // I made it all declaring at the here for to avoid confusion

    if (!isAlphanumeric(DEFAULT_FUNCTION_SIZE_MIN, DEFAULT_FUNCTION_SIZE_MAX, _data))
        return false;

    // -----

    switch (_selector) {

        case mapData::function:

            // Malloc a word, a list of characters, after carry it
            givenList.function = (char *) malloc(sizeof (char) * strlen(_data));
            strcpy(givenList.function, _data);

            break;

        case mapData::param:

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

// -----

bool Slave::checkSeparatorList(char *_data) {

    // Declare an iterators for storing last visited data
    uint16_t previousIndex = 0;

    // Declare an variable for storing done seperator
    uint16_t checkedSeparator = 0;

    for (uint16_t currentIndex = 0; currentIndex < strlen(_data); currentIndex++) {

        if (isSeparator(_data[currentIndex])) {

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

            // Update index
            _data[currentIndex] = configList.functionDelimiter;
        }
    }

    return true;
}

bool Slave::checkMapList(char *_data) {

    // Store map list index at the here
    uint16_t checkedMap = 0;

    // Store splitting data at the here
    char *mapListPointer = strtok(_data, &configList.functionDelimiter);

    while (mapListPointer != NULL) {

        // Check index on separator list size
        if (checkedMap >= DEFAULT_MAPLIST_SIZE)
            return false;

        // Only this check is enough, there is no need for further check
        if (!isMap(mapList[checkedMap++], mapListPointer))
            return false;

        mapListPointer = strtok(NULL, &configList.functionDelimiter);
    }

    return true;
}

// -----

bool Slave::decodeData(char _data[]) {

    // This makes absolute compare, whatever abs of variance must be 0 or 1
    if (abs(DEFAULT_SEPARATORLIST_SIZE - DEFAULT_MAPLIST_SIZE) > 1)
        return false;

    if (!checkSeparatorList(_data))
        return false;

    if (!checkMapList(_data))
        return false;

    // -----

    // Check that is it added before with same ID ?
    for (uint8_t index = 0; index < configList.functionListSize; index++) {
        if (strcmp(functionList[index].function, givenList.function) == 0) {

            functionList[index].pointer(givenList.paramSize, givenList.paramList);
            break;
        }
    }

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

bool Slave::setFunction(void (*pointer)(uint16_t, char**), char _function[]) {

    // Check that is it added before with same ID ?
    for (uint8_t index = 0; index < configList.functionListSize; index++)
        if (strcmp(functionList[index].function, _function) == 0)
            return false;

    if (configList.functionListSize >= DEFAULT_FUNCTIONLIST_SIZE)
        return false;

    // Update size of list index. this is our final code at this function
    configList.functionListSize++;

    functionList[configList.functionListSize - 1].pointer = pointer;
    functionList[configList.functionListSize - 1].function = _function;

    return true;
}

void Slave::onUnknownData(void (*pointer)(char[])) {

    configList.onUnknownPointer = pointer;
}

// -----

void Slave::pullData(char _data[]) {

    // First, clear recent given data
    clearGivenList();

    char data[strlen(_data)];
    strcpy(data, _data);

    if (!decodeData(_data) && configList.onUnknownPointer != NULL)
        configList.onUnknownPointer(data);
}

void Slave::pushData() {
}

// -----

/**
 * Preinstantiate Objects
 * 
 * @param -
 * @return -
 */
Slave BusProtocol = Slave();