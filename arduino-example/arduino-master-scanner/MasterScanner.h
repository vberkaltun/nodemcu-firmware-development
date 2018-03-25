/*
 * MasterScanner.h
 * I2C SCANNER - 23.03.2018
 * 
 * ==============================================================================
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
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * ==============================================================================
 */

#ifndef SCANNER_H
#define SCANNER_H

#include <stdlib.h>

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include "Wire.h"

#define DEFALUT_START_ADDRESS 0x03
#define DEFALUT_STOP_ADDRESS 0x77
#define DEFALUT_INTERVAL_MILLIS 500

class Scanner {
private:

    struct Data {
        uint8_t startAddress = DEFALUT_START_ADDRESS;
        uint8_t stopAddress = DEFALUT_STOP_ADDRESS;
        unsigned long intervalMillis = DEFALUT_INTERVAL_MILLIS;

        uint8_t connectedSlavesArray[DEFALUT_STOP_ADDRESS];
        byte connectedSlavesCount = 0;
    };

    const Data defaultData;
    Data givenData = defaultData;

    // -----

    unsigned long previousMillis = 0;

    void (*onConnected)(uint8_t[], byte);
    void (*onDisconnected)(uint8_t[], byte);
    
    void onTriggeredConnected(uint8_t _array[], byte _count);
    void onTriggeredDisconnected(uint8_t _array[], byte _count);

public:

    Scanner();

    void setRange(unsigned long _intervalMillis, uint8_t _startAddress, uint8_t _stopAddress);
    void setRange(unsigned long _intervalMillis);
    void setRange(uint8_t _startAddress, uint8_t _stopAddress);

    void resetRange();
    void scanSlaves();

    void onConnectedSlaves(void (*function)(uint8_t[], byte));
    void onDisconnectedSlaves(void (*function)(uint8_t[], byte));

    uint8_t getStartAddress();
    uint8_t getStopAddress();
    unsigned long getIntervalMillis();
    byte getConnectedSlavesCount();

    bool isConnected(uint8_t _Address);
};

extern Scanner MasterScanner;

#endif