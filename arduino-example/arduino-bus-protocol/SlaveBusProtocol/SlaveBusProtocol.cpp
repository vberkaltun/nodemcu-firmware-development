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

bool Slave::isNumeric(uint16_t _lowerBound, uint16_t _upperBound, char _data[]) {

    // Store the size of received data at the here
    uint16_t sizeofData = strlen(_data);

    // -----

    if (_data == NULL)
        return false;

    for (uint16_t index = 0; index < sizeofData; index++)
        if (isdigit(_data[0]) == 0)
            return false;

    if (atoi(_data) < _lowerBound)
        return false;

    if (atoi(_data) > _upperBound)
        return false;

    return true;
}

bool Slave::isAlphanumeric(uint16_t _lowerBound, uint16_t _upperBound, char _data[]) {

    // Store the size of received data at the here
    uint16_t sizeofData = strlen(_data);

    // -----

    if (_data == NULL)
        return false;

    if (sizeofData < _lowerBound)
        return false;

    if (sizeofData > _upperBound)
        return false;

    // Check function ID, type is alphanumeric
    for (uint16_t index = 0; index < sizeofData; index++)
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

    // Store the size of received data at the here
    uint16_t sizeofData = strlen(_data);

    // -----

    switch (_selector) {

        case mapData::function:

            if (!isAlphanumeric(DEFAULT_FUNCTION_SIZE_MIN, DEFAULT_FUNCTION_SIZE_MAX, _data))
                return false;

            // Malloc a word, a list of characters, after carry it
            receivedList.function = (char *) malloc(sizeof (char) * sizeofData);
            strcpy(receivedList.function, _data);

            break;

        case mapData::param:

            if (!isAlphanumeric(DEFAULT_FUNCTION_SIZE_MIN, DEFAULT_FUNCTION_SIZE_MAX, _data))
                return false;

            // Malloc and realloc a sentence,  a list of words
            if (receivedList.paramList == NULL)
                receivedList.paramList = (char **) malloc(sizeof (char *) * (++receivedList.paramSize));
            else
                receivedList.paramList = (char **) realloc(receivedList.paramList, sizeof (char *) * (++receivedList.paramSize));

            // Malloc and realloc a word, a list of characters, after carry it
            receivedList.paramList[receivedList.paramSize - 1] = (char *) malloc(sizeof (char) * 32);
            strcpy(receivedList.paramList[receivedList.paramSize - 1], _data);

            break;

        default:
            return false;
    }

    return true;
}

bool Slave::isFunction(functionData::typeData _selector, uint8_t _index) {

    switch (_selector) {

        case functionData::is_INT8_T:
            functionList[_index].pointer.INT8_T(receivedList.paramSize, receivedList.paramList);
            break;

        case functionData::is_INT16_T:
            functionList[_index].pointer.INT16_T(receivedList.paramSize, receivedList.paramList);
            break;

        case functionData::is_INT32_T:
            functionList[_index].pointer.INT32_T(receivedList.paramSize, receivedList.paramList);
            break;

        case functionData::is_UINT8_T:
            functionList[_index].pointer.UINT8_T(receivedList.paramSize, receivedList.paramList);
            break;

        case functionData::is_UINT16_T:
            functionList[_index].pointer.UINT16_T(receivedList.paramSize, receivedList.paramList);
            break;

        case functionData::is_UINT32_T:
            functionList[_index].pointer.UINT32_T(receivedList.paramSize, receivedList.paramList);
            break;

        case functionData::is_CHAR:
            functionList[_index].pointer.CHAR(receivedList.paramSize, receivedList.paramList);
            break;

        case functionData::is_BOOL:
            functionList[_index].pointer.BOOL(receivedList.paramSize, receivedList.paramList);
            break;

        case functionData::is_FLOAT:
            functionList[_index].pointer.FLOAT(receivedList.paramSize, receivedList.paramList);
            break;

        case functionData::is_VOID:
            functionList[_index].pointer.VOID(receivedList.paramSize, receivedList.paramList);
            break;

        default:
            return false;
    }

    return true;
}

// -----

bool Slave::checkSeparatorList(char *_data) {

    // Store the size of received data at the here
    uint16_t sizeofData = strlen(_data);

    // -----

    // Declare an iterators for storing last visited data
    uint16_t previousIndex = 0;

    // Declare an variable for storing done seperator
    uint16_t checkedSeparator = 0;

    for (uint16_t currentIndex = 0; currentIndex < sizeofData; currentIndex++) {

        if (isSeparator(_data[currentIndex])) {

            // Check index on separator list size
            if (checkedSeparator >= DEFAULT_SEPARATORLIST_SIZE)
                return false;

            // Check value on received index in separator list
            if (separatorList[checkedSeparator++] != _data[currentIndex])
                return false;

            // Check whether two separators on back-to-back
            if (currentIndex - previousIndex == 1)
                return false;

            // Update index
            previousIndex = currentIndex;

            // Update char at received index
            // Do not remove this code, It is related with next step (checkMapList)
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

bool Slave::checkFunctionList(char *_data) {

    // Check that is it added before with same ID ?
    for (uint8_t index = 0; index < configList.functionListCount; index++)
        if (strcmp(functionList[index].function, _data) == 0)
            return isFunction(functionList[index].type, index);

    return false;
}

// -----

bool Slave::decodeData(char _data[]) {

    // This makes absolute compare, whatever abs of variance must be 0 or 1
    if (abs(DEFAULT_SEPARATORLIST_SIZE - DEFAULT_MAPLIST_SIZE) > 1)
        return false;

    // -----

    if (!checkSeparatorList(_data))
        return false;

    if (!checkMapList(_data))
        return false;

    if (!checkFunctionList(receivedList.function))
        return false;

    return true;
}

void Slave::clearReceivedData() {

    // Clear recent stored data, We will fill again
    receivedList.paramSize = 0;

    // IMPORTANT NOTICE: seems so confused, right? 
    // When you free up a pointer, you can not use it again anymore 
    // But when you make reassigning NULL to a pointer after the freeing up, 
    // You can this pointer again very well

    free(receivedList.paramList);
    receivedList.paramList = NULL;

    free(receivedList.function);
    receivedList.function = NULL;
}

void Slave::clearGivenData() {

    givenList.separatedSize = 0;

    // IMPORTANT NOTICE: seems so confused, right? 
    // When you free up a pointer, you can not use it again anymore 
    // But when you make reassigning NULL to a pointer after the freeing up, 
    // You can this pointer again very well

    free(givenList.separated);
    givenList.separated = NULL;

    free(givenList.combined);
    givenList.combined = NULL;
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

void Slave::unknownData(void (*pointer)(char[])) {

    configList.onUnknownPointer = pointer;
}

void Slave::analyzeData(char _data[]) {

    // Store the size of received data at the here
    uint16_t sizeofData = strlen(_data);

    // -----

    char unknownData[sizeofData];
    strcpy(unknownData, _data);

    if (!decodeData(_data) && configList.onUnknownPointer != NULL)
        configList.onUnknownPointer(unknownData);

    // IMPORTANT NOTICE: At the end of each process 
    // The memory must be cleaned absolutely. 
    // Otherwise, memory corruption may occur and data may be damaged
    clearReceivedData();
}

bool Slave::setFunction(int8_t(*pointer)(uint16_t, char**), char _function[]) {

    // Check that is it added before with same ID ?
    for (uint16_t index = 0; index < configList.functionListCount; index++)
        if (strcmp(functionList[index].function, _function) == 0)
            return false;

    if (configList.functionListCount >= DEFAULT_FUNCTIONLIST_SIZE)
        return false;

    functionList[configList.functionListCount].type = functionData::is_INT8_T;
    functionList[configList.functionListCount].pointer.INT8_T = pointer;
    functionList[configList.functionListCount].function = _function;

    // Update size of list index. this is our final code at this function
    configList.functionListCount++;

    return true;
}

bool Slave::setFunction(int16_t(*pointer)(uint16_t, char**), char _function[]) {

    // Check that is it added before with same ID ?
    for (uint16_t index = 0; index < configList.functionListCount; index++)
        if (strcmp(functionList[index].function, _function) == 0)
            return false;

    if (configList.functionListCount >= DEFAULT_FUNCTIONLIST_SIZE)
        return false;

    functionList[configList.functionListCount].type = functionData::is_INT16_T;
    functionList[configList.functionListCount].pointer.INT16_T = pointer;
    functionList[configList.functionListCount].function = _function;

    // Update size of list index. this is our final code at this function
    configList.functionListCount++;

    return true;
}

bool Slave::setFunction(int32_t(*pointer)(uint16_t, char**), char _function[]) {

    // Check that is it added before with same ID ?
    for (uint16_t index = 0; index < configList.functionListCount; index++)
        if (strcmp(functionList[index].function, _function) == 0)
            return false;

    if (configList.functionListCount >= DEFAULT_FUNCTIONLIST_SIZE)
        return false;

    functionList[configList.functionListCount].type = functionData::is_INT32_T;
    functionList[configList.functionListCount].pointer.INT32_T = pointer;
    functionList[configList.functionListCount].function = _function;

    // Update size of list index. this is our final code at this function
    configList.functionListCount++;

    return true;
}

bool Slave::setFunction(uint8_t(*pointer)(uint16_t, char**), char _function[]) {

    // Check that is it added before with same ID ?
    for (uint16_t index = 0; index < configList.functionListCount; index++)
        if (strcmp(functionList[index].function, _function) == 0)
            return false;

    if (configList.functionListCount >= DEFAULT_FUNCTIONLIST_SIZE)
        return false;

    functionList[configList.functionListCount].type = functionData::is_UINT8_T;
    functionList[configList.functionListCount].pointer.UINT8_T = pointer;
    functionList[configList.functionListCount].function = _function;

    // Update size of list index. this is our final code at this function
    configList.functionListCount++;

    return true;
}

bool Slave::setFunction(uint16_t(*pointer)(uint16_t, char**), char _function[]) {

    // Check that is it added before with same ID ?
    for (uint16_t index = 0; index < configList.functionListCount; index++)
        if (strcmp(functionList[index].function, _function) == 0)
            return false;

    if (configList.functionListCount >= DEFAULT_FUNCTIONLIST_SIZE)
        return false;

    functionList[configList.functionListCount].type = functionData::is_UINT16_T;
    functionList[configList.functionListCount].pointer.UINT16_T = pointer;
    functionList[configList.functionListCount].function = _function;

    // Update size of list index. this is our final code at this function
    configList.functionListCount++;
    return true;
}

bool Slave::setFunction(uint32_t(*pointer)(uint16_t, char**), char _function[]) {

    // Check that is it added before with same ID ?
    for (uint16_t index = 0; index < configList.functionListCount; index++)
        if (strcmp(functionList[index].function, _function) == 0)
            return false;

    if (configList.functionListCount >= DEFAULT_FUNCTIONLIST_SIZE)
        return false;

    functionList[configList.functionListCount].type = functionData::is_UINT32_T;
    functionList[configList.functionListCount].pointer.UINT32_T = pointer;
    functionList[configList.functionListCount].function = _function;

    // Update size of list index. this is our final code at this function
    configList.functionListCount++;

    return true;
}

bool Slave::setFunction(char* (*pointer)(uint16_t, char**), char _function[]) {

    // Check that is it added before with same ID ?
    for (uint16_t index = 0; index < configList.functionListCount; index++)
        if (strcmp(functionList[index].function, _function) == 0)
            return false;

    if (configList.functionListCount >= DEFAULT_FUNCTIONLIST_SIZE)
        return false;

    functionList[configList.functionListCount].type = functionData::is_CHAR;
    functionList[configList.functionListCount].pointer.CHAR = pointer;
    functionList[configList.functionListCount].function = _function;

    // Update size of list index. this is our final code at this function
    configList.functionListCount++;

    return true;
}

bool Slave::setFunction(bool (*pointer)(uint16_t, char**), char _function[]) {

    // Check that is it added before with same ID ?
    for (uint16_t index = 0; index < configList.functionListCount; index++)
        if (strcmp(functionList[index].function, _function) == 0)
            return false;

    if (configList.functionListCount >= DEFAULT_FUNCTIONLIST_SIZE)
        return false;

    functionList[configList.functionListCount].type = functionData::is_BOOL;
    functionList[configList.functionListCount].pointer.BOOL = pointer;
    functionList[configList.functionListCount].function = _function;

    // Update size of list index. this is our final code at this function
    configList.functionListCount++;

    return true;
}

bool Slave::setFunction(float (*pointer)(uint16_t, char**), char _function[]) {

    // Check that is it added before with same ID ?
    for (uint16_t index = 0; index < configList.functionListCount; index++)
        if (strcmp(functionList[index].function, _function) == 0)
            return false;

    if (configList.functionListCount >= DEFAULT_FUNCTIONLIST_SIZE)
        return false;

    functionList[configList.functionListCount].type = functionData::is_FLOAT;
    functionList[configList.functionListCount].pointer.FLOAT = pointer;
    functionList[configList.functionListCount].function = _function;

    // Update size of list index. this is our final code at this function
    configList.functionListCount++;

    return true;
}

bool Slave::setFunction(void (*pointer)(uint16_t, char**), char _function[]) {

    // Check that is it added before with same ID ?
    for (uint16_t index = 0; index < configList.functionListCount; index++)
        if (strcmp(functionList[index].function, _function) == 0)
            return false;

    if (configList.functionListCount >= DEFAULT_FUNCTIONLIST_SIZE)
        return false;

    functionList[configList.functionListCount].type = functionData::is_VOID;
    functionList[configList.functionListCount].pointer.VOID = pointer;
    functionList[configList.functionListCount].function = _function;

    // Update size of list index. this is our final code at this function
    configList.functionListCount++;

    return true;
}

// -----

/**
 * Preinstantiate Objects
 * 
 * @param -
 * @return -
 */
Slave BusProtocol = Slave();