/*
 * Serializer.h
 * SERIALIZER - 17.04.2018
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

#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <stdlib.h>
#include <string.h>

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

class Serializer {
private:

    struct decodedData {
        unsigned short sizeofDelimiter = 0;
        unsigned short sizeofGivenData = 0;
        unsigned short sizeofResultData = 0;
        char *delimiter = NULL;
        char *givenData = NULL;
        char **resultData = NULL;
    } decodedList;

    struct encodedData {
        unsigned short sizeofDelimiter = 0;
        unsigned short sizeofResultData = 0;
        unsigned short sizeofGivenData = 0;
        char *delimiter = NULL;
        char **givenData = NULL;
        char *resultData = NULL;

    } encodedList;

    // -----

    unsigned short calculateMaximumLineWidth();

    void clearDecodedList(bool isAllData);
    void clearEncodedList(bool isAllData);

    void fillDecodedList();
    void fillEncodedList();

    bool decodeData();
    bool encodeData();

public:

    Serializer();

    char **decode(unsigned short sizeofDelimiter, char delimiter[], unsigned short sizeofGivenData, char givenData[]);
    char *encode(unsigned short sizeofDelimiter, char delimiter[], unsigned short sizeofGivenData, char *givenData[]);
};

extern Serializer Serialization;

#endif