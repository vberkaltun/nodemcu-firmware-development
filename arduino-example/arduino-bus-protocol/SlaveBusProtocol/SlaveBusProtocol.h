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

#define DEFAULT_FUNCTIONID_SIZE_MIN 3
#define DEFAULT_FUNCTIONID_SIZE_MAX 64

#define DEFAULT_PATTERN_SIZE_MIN 4
#define DEFAULT_PATTERN_SIZE_MAX 32

#define DEFAULT_SEPARATORLIST_SIZE 9
#define DEFAULT_MAPLIST_SIZE 8
#define DEFAULT_FUNCTIONLIST_SIZE 32

class Slave {
private:

    struct givenData {
        uint8_t queueID;
        uint16_t crcChecksum;
        bool returnRequest;
        char* functionID;
        char* paramList = NULL;
    } givenList;

    enum separatorData {
        file = 0x1C,
        group = 0x1D,
        record = 0x1E,
        unit = 0x1F
    } separatorList[DEFAULT_SEPARATORLIST_SIZE] = {
        Slave::separatorData::file,
        Slave::separatorData::record,
        Slave::separatorData::record,
        Slave::separatorData::group,
        Slave::separatorData::record,
        Slave::separatorData::unit,
        Slave::separatorData::unit,
        Slave::separatorData::unit,
        Slave::separatorData::file
    };

    enum mapData {
        queue,
        crc,
        request,
        function,
        param
    } mapList[DEFAULT_SEPARATORLIST_SIZE] = {
        Slave::mapData::queue,
        Slave::mapData::crc,
        Slave::mapData::request,
        Slave::mapData::function,
        Slave::mapData::param,
        Slave::mapData::param,
        Slave::mapData::param,
        Slave::mapData::param
    };

    // -----

    struct configData {
        uint8_t deviceAddress;
        char* deviceBrand;
        char* deviceID;
        char* deviceVersion;
        bool setFlag = false;
        uint8_t functionListCount = 0;
    } configList;

    struct functionData {

        enum typeData {
            is_int8_t,
            is_int16_t,
            is_int32_t,
            is_uint8_t,
            is_uint16_t,
            is_uint32_t,
            is_char,
            is_bool,
            is_float,
            is_void,
        } type;

        union pointerData {
            int8_t(*int8_tValue)(uint16_t, char[]);
            int16_t(*int16_tValue)(uint16_t, char[]);
            int32_t(*int32_tValue)(uint16_t, char[]);
            uint8_t(*uint8_tValue)(uint16_t, char[]);
            uint16_t(*uint16_tValue)(uint16_t, char[]);
            uint32_t(*uint32_tValue)(uint16_t, char[]);
            char* (*charValue)(uint16_t, char[]);
            bool (*boolValue)(uint16_t, char[]);
            float (*floatValue)(uint16_t, char[]);
            void (*voidValue)(uint16_t, char[]);
        } pointer;

        char* functionID;

    } *functionList;

    // -----

    void (*onUnknown)(uint16_t, char[]) = NULL;

    bool checkSeparator(char _map);
    bool checkMap(Slave::mapData _selector, uint16_t _length, char _data[]);

    bool checkSeparatorList(char _data[]);
    bool checkMapList(char _data[]);
    bool checkConfigList(char _data[]);
    bool checkFunctionList(char _data[]);

    bool decodeData(char _data[]);
    bool encodeData(); // todo

public:

    Slave();

    bool setFunction(int8_t(*pointer)(uint16_t, char[]), char _functionID[]);
    bool setFunction(int16_t(*pointer)(uint16_t, char[]), char _functionID[]);
    bool setFunction(int32_t(*pointer)(uint16_t, char[]), char _functionID[]);
    bool setFunction(uint8_t(*pointer)(uint16_t, char[]), char _functionID[]);
    bool setFunction(uint16_t(*pointer)(uint16_t, char[]), char _functionID[]);
    bool setFunction(uint32_t(*pointer)(uint16_t, char[]), char _functionID[]);
    bool setFunction(char*(*pointer)(uint16_t, char[]), char _functionID[]);
    bool setFunction(bool (*pointer)(uint16_t, char[]), char _functionID[]);
    bool setFunction(float (*pointer)(uint16_t, char[]), char _functionID[]);
    bool setFunction(void (*pointer)(uint16_t, char[]), char _functionID[]);
    bool setConfig(uint8_t _deviceAddress, char _deviceBrand[], char _deviceID[], char _deviceVersion[]);

    void pullData(char _data[]);
    void pushData(); // todo

    void onUnknownData(void (*pointer)(uint16_t, char[]));
};

extern Slave BusProtocol;

#endif