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

#define DEFAULT_SEPARATORLIST_SIZE 9
#define DEFAULT_MAPLIST_SIZE 8
#define DEFAULT_FUNCTIONLIST_SIZE 32

class Slave {
private:

    struct givenData {
        uint8_t paramSize = 0;
        char **paramList = NULL;
        char *function = NULL;
    } givenList;

    struct configData {
        void (*onUnknownPointer)(char[]) = NULL;
        uint8_t functionListSize;
        char functionDelimiter = '\n';
    } configList;

    // -----

    enum separatorData {
        file = 0x1C,
        group = 0x1D,
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
        param
    } mapList[DEFAULT_SEPARATORLIST_SIZE] = {
        mapData::param,
        mapData::param,
        mapData::function,
        mapData::param,
        mapData::param,
        mapData::param,
        mapData::param
    };

    struct functionData {
        void (*pointer)(uint16_t, char**);
        char *function;

    } functionList[DEFAULT_FUNCTIONLIST_SIZE];

    // -----

    void clearGivenList();

    bool isNumeric(uint8_t _lowerBound, uint8_t _upperBound, char _data[]);
    bool isAlphanumeric(uint8_t _lowerBound, uint8_t _upperBound, char _data[]);

    bool isSeparator(char _selector);
    bool isMap(mapData _selector, char _data[]);

    bool checkSeparatorList(char *_data);
    bool checkMapList(char *_data);

    bool decodeData(char _data[]);
    bool encodeData(); // todo

public:

    Slave();

    bool setFunction(void (*pointer)(uint16_t, char**), char _function[]);

    void pullData(char _data[]);
    void pushData(); // todo

    void onUnknownData(void (*pointer)(char[]));
};

extern Slave BusProtocol;

#endif