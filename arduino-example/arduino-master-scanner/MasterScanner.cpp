/*
 * MasterScanner.cpp
 * MASTER SCANNER - 23.03.2018
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

#include "MasterScanner.h"

void Scanner::onTriggeredConnected(uint8_t _array[], byte _count) {

    if (_count != 0) {

        // don't bother if user hasn't registered a callback
        if (!onConnected) {
            return;
        }

        onConnected(_array, _count);
    }
}

void Scanner::onTriggeredDisconnected(uint8_t _array[], byte _count) {

    if (_count != 0) {

        // don't bother if user hasn't registered a callback
        if (!onDisconnected) {
            return;
        }

        onDisconnected(_array, _count);
    }
}

void Scanner::cleanRange(uint8_t _startAddress, uint8_t _stopAddress, uint8_t _array[]) {

    for (_startAddress; _startAddress <= _stopAddress; _stopAddress++)
        givenData.connectedSlavesArray[_startAddress] = NULL;
}

// -----

/**
 * Constructors 
 * 
 * @param -
 * @return -
 */
Scanner::Scanner() {
}

// -----

/**
 * Changes default scanning range with new range, additional changes delay
 * 
 * @param Delay, start and stop address of range
 * @return -
 */
void Scanner::setRange(unsigned long _intervalMillis, uint8_t _startAddress, uint8_t _stopAddress) {

    this->setRange(_intervalMillis);
    this->setRange(_startAddress, _stopAddress);
}

/**
 * Changes default scanning range with new range
 * 
 * @param Delay range
 * @return -
 */
void Scanner::setRange(unsigned long _intervalMillis) {

    if (_intervalMillis < 0)
        givenData.intervalMillis = defaultData.intervalMillis;
    else
        givenData.intervalMillis = _intervalMillis;
}

/**
 * Changes default scanning range with new range
 * 
 * @param Start and stop address of range
 * @return -
 */
void Scanner::setRange(uint8_t _startAddress, uint8_t _stopAddress) {

    if (_startAddress <= _stopAddress && _startAddress >= defaultData.startAddress && _stopAddress <= defaultData.stopAddress) {

        if (_startAddress > givenData.startAddress)
            this->cleanRange(givenData.startAddress, _startAddress - 1, givenData.connectedSlavesArray);

        if (_stopAddress < givenData.stopAddress)
            this->cleanRange(_stopAddress + 1, givenData.stopAddress, givenData.connectedSlavesArray);

        givenData.startAddress = _startAddress;
        givenData.stopAddress = _stopAddress;

    } else
        this->setRange(defaultData.startAddress, defaultData.stopAddress);
}

/**
 * Resets current scanning range with default range
 * 
 * @param Start and stop address of range
 * @return -
 */
void Scanner::resetRange() {

    this->setRange(defaultData.intervalMillis, defaultData.startAddress, defaultData.stopAddress);
}

// -----

/**
 * Scans connected devices with the given range
 * 
 * @param -
 * @return -
 */
void Scanner::scanSlaves() {

    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= givenData.intervalMillis) {

        // save the last time you blinked the LED
        previousMillis = currentMillis;

        // -----

        uint8_t *currentConnectedSlavesArray = NULL;
        uint8_t *currentDisconnectedSlavesArray = NULL;

        byte currentConnectedSlavesCount = 0;
        byte currentDisconnectedSlavesCount = 0;

        // -----

        uint8_t currentAddress;

        // addresses 0x00 through 0x77
        for (currentAddress = givenData.startAddress; currentAddress <= givenData.stopAddress; currentAddress++) {

            Wire.beginTransmission(currentAddress);

            if (Wire.endTransmission() == 0) {

                if (givenData.connectedSlavesArray[currentAddress] == NULL) {
                    givenData.connectedSlavesArray[currentAddress] = currentAddress;

                    if (currentConnectedSlavesArray == NULL)
                        currentConnectedSlavesArray = (uint8_t*) malloc(sizeof (uint8_t)*(++currentConnectedSlavesCount));
                    else
                        currentConnectedSlavesArray = (uint8_t*) realloc(currentConnectedSlavesArray, sizeof (uint8_t)*(++currentConnectedSlavesCount));

                    currentConnectedSlavesArray[currentConnectedSlavesCount - 1] = currentAddress;
                }

            } else {

                if (givenData.connectedSlavesArray[currentAddress] != NULL) {
                    givenData.connectedSlavesArray[currentAddress] = NULL;

                    if (currentDisconnectedSlavesArray == NULL)
                        currentDisconnectedSlavesArray = (uint8_t*) malloc(sizeof (uint8_t)*(++currentDisconnectedSlavesCount));
                    else
                        currentDisconnectedSlavesArray = (uint8_t*) realloc(currentDisconnectedSlavesArray, sizeof (uint8_t)*(++currentDisconnectedSlavesCount));

                    currentDisconnectedSlavesArray[currentDisconnectedSlavesCount - 1] = currentAddress;
                }
            }
        }

        onTriggeredConnected(currentConnectedSlavesArray, currentConnectedSlavesCount);
        onTriggeredDisconnected(currentDisconnectedSlavesArray, currentDisconnectedSlavesCount);

        // always free at the end
        free(currentConnectedSlavesArray);
        free(currentDisconnectedSlavesArray);
    }
}

// -----

/**
 * Triggers when at least one slave is connected
 * 
 * @param A function that has two parameters of array and count
 * @return -
 */
void Scanner::onConnectedSlaves(void (*pointer)(uint8_t[], byte)) {

    onConnected = pointer;
}

/**
 * Triggers when at least one slave is disconnected
 * 
 * @param A function that has two parameters of array and count
 * @return -
 */
void Scanner::onDisconnectedSlaves(void (*pointer)(uint8_t[], byte)) {

    onDisconnected = pointer;
}

// -----

/**
 * Gets start address of range
 * 
 * @param -
 * @return Start address of range
 */
uint8_t Scanner::getStartAddress() {

    return givenData.startAddress;
}

/**
 * Gets stop address of range
 * 
 * @param -
 * @return Stop address of range
 */
uint8_t Scanner::getStopAddress() {

    return givenData.stopAddress;
}

/**
 * Gets scanning frequency
 * 
 * @param -
 * @return Scanning frequency
 */
unsigned long Scanner::getIntervalMillis() {

    return givenData.intervalMillis;
}

/**
 * Gets connected slaves count
 * 
 * @param -
 * @return Connected slaves count
 */
byte Scanner::getConnectedSlavesCount() {

    byte currentConnectedSlavesCount = 0;
    uint8_t currentAddress;

    // addresses 0x00 through 0x77
    for (currentAddress = givenData.startAddress; currentAddress <= givenData.stopAddress; currentAddress++)
        if (givenData.connectedSlavesArray[currentAddress] != NULL) currentConnectedSlavesCount++;

    return currentConnectedSlavesCount;
}

/**
 * Checks if specified address is online on bus
 * 
 * @param Address of specified device
 * @return Connected or not Connected
 */
bool Scanner::isConnected(uint8_t _address) {

    bool isConnectedFlag = 0;
    uint8_t currentAddress;

    // addresses 0x00 through 0x77
    for (currentAddress = givenData.startAddress; currentAddress <= givenData.stopAddress; currentAddress++) {
        if (givenData.connectedSlavesArray[currentAddress] == _address) {

            isConnectedFlag = true;
            break;
        }
    }

    return isConnectedFlag;
}

// -----

/**
 * Preinstantiate Objects
 * 
 * @param -
 * @return -
 */
Scanner MasterScanner = Scanner();
