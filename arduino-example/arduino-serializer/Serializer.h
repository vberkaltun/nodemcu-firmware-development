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
        char *delimiter = NULL;
        int32_t sizeofResultData = 0;
        char *givenData = NULL;
        char **resultData = NULL;
    } decodedList;

    struct encodedData {
        char *delimiter = NULL;
        int32_t sizeofResultData = 0;
        int32_t sizeofGivenData = 0;
        char *resultData = NULL;
        char **givenData = NULL;
    } encodedList;

    // -----

    int32_t calculateMaximumLineWidth();

    void clearDecodedList();
    void clearEncodedList();

    void fillDecodedList();
    void fillEncodedList();

    bool decodeData();
    bool encodeData();

public:

    Serializer();

    char **decode(const char *delimiter, char *givenData);
    char *encode(const char *delimiter, char **givenData);
};

extern Serializer Serialization;

#endif