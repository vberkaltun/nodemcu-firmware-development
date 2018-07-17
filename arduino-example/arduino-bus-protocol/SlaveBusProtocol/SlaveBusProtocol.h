/*
 * SlaveBusProtocol.h
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

#ifndef SLAVEBUSPROTOCOL_H
#define SLAVEBUSPROTOCOL_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "Wire.h"

// IMPORTANT NOTICE: You can change the following variables depending on your objectives
// Depending on the changes you made, you should also update the variables
// Otherwise, you can get ordinary compilation or memory failure

#define DEFAULT_FUNCTION_SIZE_MIN 1
#define DEFAULT_FUNCTION_SIZE_MAX 32

#define DEFAULT_SEPARATORLIST_SIZE 8
#define DEFAULT_MAPLIST_SIZE 7
#define DEFAULT_FUNCTIONLIST_SIZE 32

class Slave {
private:

    struct receivedData {
        uint16_t paramSize = 0;
        char **paramList = NULL;
        char *function = NULL;
    } receivedList;

    struct givenData {
        uint16_t separatedSize = 0;
        char **separated = NULL;
        char *combined = NULL;
        char *function = NULL;
    } givenList;

    struct configData {
        void (*onUnknownPointer)(char[]) = NULL;
        uint16_t functionListCount = 0;
        char functionDelimiter = '\n';
    } configList;

    // -----

    enum separatorData {
        file = 0x1C,
        group = 0x1D,
        idle = 0x1B
    } separatorList[DEFAULT_SEPARATORLIST_SIZE] = {
        separatorData::file,
        separatorData::group,
        separatorData::group,
        separatorData::group,
        separatorData::group,
        separatorData::group,
        separatorData::group,
        separatorData::file
    };

    enum mapData {
        function,
        param,
    } mapList[DEFAULT_MAPLIST_SIZE] = {
        mapData::param,
        mapData::param,
        mapData::function,
        mapData::param,
        mapData::param,
        mapData::param,
        mapData::param
    };

    struct functionData {

        enum typeData {
            is_INT8_T,
            is_INT16_T,
            is_INT32_T,
            is_UINT8_T,
            is_UINT16_T,
            is_UINT32_T,
            is_CHAR,
            is_BOOL,
            is_FLOAT,
            is_VOID
        } type;

        union pointerData {
            int8_t(*INT8_T)(uint16_t, char**);
            int16_t(*INT16_T)(uint16_t, char**);
            int32_t(*INT32_T)(uint16_t, char**);
            uint8_t(*UINT8_T)(uint16_t, char**);
            uint16_t(*UINT16_T)(uint16_t, char**);
            uint32_t(*UINT32_T)(uint16_t, char**);
            char* (*CHAR)(uint16_t, char**);
            bool (*BOOL)(uint16_t, char**);
            float (*FLOAT)(uint16_t, char**);
            void (*VOID)(uint16_t, char**);
        } pointer;

        char* function;

    } functionList[DEFAULT_FUNCTIONLIST_SIZE];

    // -----

    bool isNumeric(uint16_t _lowerBound, uint16_t _upperBound, char _data[]);
    bool isAlphanumeric(uint16_t _lowerBound, uint16_t _upperBound, char _data[]);

    bool isSeparator(char _selector);
    bool isMap(mapData _selector, char _data[]);
    bool isFunction(functionData::typeData _selector, uint8_t _index);

    bool checkSeparatorList(char *_data);
    bool checkMapList(char *_data);
    bool checkFunctionList(char *_data);

    bool decodeData(char _data[]);

    void clearReceivedData();
    void clearGivenData();

public:

    Slave();

    void unknownData(void (*pointer)(char[]));
    void analyzeData(char _data[]);

    bool setFunction(int8_t(*pointer)(uint16_t, char**), char _function[]);
    bool setFunction(int16_t(*pointer)(uint16_t, char**), char _function[]);
    bool setFunction(int32_t(*pointer)(uint16_t, char**), char _function[]);
    bool setFunction(uint8_t(*pointer)(uint16_t, char**), char _function[]);
    bool setFunction(uint16_t(*pointer)(uint16_t, char**), char _function[]);
    bool setFunction(uint32_t(*pointer)(uint16_t, char**), char _function[]);
    bool setFunction(char*(*pointer)(uint16_t, char**), char _function[]);
    bool setFunction(bool (*pointer)(uint16_t, char**), char _function[]);
    bool setFunction(float (*pointer)(uint16_t, char**), char _function[]);
    bool setFunction(void (*pointer)(uint16_t, char**), char _function[]);
};

extern Slave BusProtocol;

#endif