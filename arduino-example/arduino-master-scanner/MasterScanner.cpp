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

bool Scanner::checkRange(uint8_t _startAddress, uint8_t _stopAddress) {

    if (_startAddress > _stopAddress)
        return false;

    if (_startAddress < defaultData.startAddress)
        return false;

    if (_stopAddress > defaultData.stopAddress)
        return false;

    return true;
}

uint8_t *Scanner::cleanRange(uint8_t _array[], uint8_t _startAddress, uint8_t _stopAddress) {

    for (uint8_t address = _startAddress; address <= _stopAddress; address++)
        givenData.connectedSlavesArray[address] = NULL;

    return _array;
}

uint8_t *Scanner::fillArray(uint8_t _array[], uint8_t _address, byte _count) {

    if (_array == NULL)
        _array = (uint8_t*) malloc(sizeof (uint8_t) * _count);
    else
        _array = (uint8_t*) realloc(_array, sizeof (uint8_t) * _count);

    _array[_count - 1] = _address;

    return _array;
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
 * Changes default scanning range with new range
 * 
 * @param Start and stop address of range
 * @return -
 */
bool Scanner::setRange(uint8_t _startAddress, uint8_t _stopAddress) {

    bool setRangeFlag = false;

    if (checkRange(_startAddress, _stopAddress)) {

        if (_startAddress > givenData.startAddress)
            givenData.connectedSlavesArray = this->cleanRange(givenData.connectedSlavesArray, givenData.startAddress, _startAddress - 1);

        if (_stopAddress < givenData.stopAddress)
            givenData.connectedSlavesArray = this->cleanRange(givenData.connectedSlavesArray, _stopAddress + 1, givenData.stopAddress);

        givenData.startAddress = _startAddress;
        givenData.stopAddress = _stopAddress;
        setRangeFlag = true;
    }

    return setRangeFlag;
}

/**
 * Resets current scanning range with default range
 * 
 * @param Start and stop address of range
 * @return -
 */
void Scanner::resetRange() {

    this->setRange(defaultData.startAddress, defaultData.stopAddress);
}

// -----

/**
 * Scans connected devices with the given range
 * 
 * @param -
 * @return -
 */
void Scanner::scanSlaves() {

    uint8_t *currentConnectedSlavesArray = NULL;
    uint8_t *currentDisconnectedSlavesArray = NULL;

    byte currentConnectedSlavesCount = 0;
    byte currentDisconnectedSlavesCount = 0;

    // -----

    // addresses 0x00 through 0x77
    for (uint8_t currentAddress = givenData.startAddress; currentAddress <= givenData.stopAddress; currentAddress++) {

        Wire.beginTransmission(currentAddress);

        if (Wire.endTransmission() == 0) {

            if (givenData.connectedSlavesArray[currentAddress] == NULL) {
                givenData.connectedSlavesArray[currentAddress] = currentAddress;
                givenData.connectedSlavesCount++;

                currentConnectedSlavesArray = this->fillArray(currentConnectedSlavesArray, currentAddress, ++currentConnectedSlavesCount);
            }

        } else {

            if (givenData.connectedSlavesArray[currentAddress] != NULL) {
                givenData.connectedSlavesArray[currentAddress] = NULL;
                givenData.connectedSlavesCount--;

                currentDisconnectedSlavesArray = this->fillArray(currentDisconnectedSlavesArray, currentAddress, ++currentDisconnectedSlavesCount);
            }
        }
    }

    onTriggeredConnected(currentConnectedSlavesArray, currentConnectedSlavesCount);
    onTriggeredDisconnected(currentDisconnectedSlavesArray, currentDisconnectedSlavesCount);

    // always free at the end
    free(currentConnectedSlavesArray);
    free(currentDisconnectedSlavesArray);
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
 * Gets connected slaves count
 * 
 * @param -
 * @return Connected slaves count
 */
byte Scanner::getConnectedSlavesCount() {

    return givenData.connectedSlavesCount;
}

/**
 * Checks if specified address is online on bus
 * 
 * @param Address of specified device
 * @return Connected or not Connected
 */
bool Scanner::isConnected(uint8_t _address) {

    bool isConnectedFlag = false;

    // addresses 0x00 through 0x77
    for (uint8_t currentAddress = givenData.startAddress; currentAddress <= givenData.stopAddress; currentAddress++) {
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