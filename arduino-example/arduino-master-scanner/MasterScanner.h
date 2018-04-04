/*
 * MasterScanner.h
 * I2C SCANNER - 23.03.2018
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

#ifndef MASTERSCANNER_H
#define MASTERSCANNER_H

#include <stdlib.h>

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include "Wire.h"

#define DEFAULT_START_ADDRESS 0x03
#define DEFAULT_STOP_ADDRESS 0x77

class Scanner {
private:

    struct data {
        uint8_t startAddress = DEFAULT_START_ADDRESS;
        uint8_t stopAddress = DEFAULT_STOP_ADDRESS;

        uint8_t connectedSlavesArray[DEFAULT_STOP_ADDRESS + 1];
        byte connectedSlavesCount = 0;
    };

    const data defaultData;
    data givenData = defaultData;

    void (*onConnected)(uint8_t[], byte) = NULL;
    void (*onDisconnected)(uint8_t[], byte) = NULL;

    // -----

    void onTriggeredConnected(uint8_t _array[], byte _count);
    void onTriggeredDisconnected(uint8_t _array[], byte _count);

    bool checkRange(uint8_t _startAddress, uint8_t _stopAddress);
    void cleanRange(uint8_t _array[], uint8_t _startAddress, uint8_t _stopAddress);
    uint8_t *fillArray(uint8_t _array[], uint8_t _address, byte _count);

public:

    Scanner();

    void setRange(uint8_t _startAddress, uint8_t _stopAddress);
    void resetRange();

    void scanSlaves();

    void onConnectedSlaves(void (*pointer)(uint8_t[], byte));
    void onDisconnectedSlaves(void (*pointer)(uint8_t[], byte));

    uint8_t getStartAddress();
    uint8_t getStopAddress();
    byte getConnectedSlavesCount();
    bool isConnected(uint8_t _address);
};

extern Scanner MasterScanner;

#endif