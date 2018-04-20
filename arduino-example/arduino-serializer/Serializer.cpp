/*
 * Serializer.cpp
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

#include "Serializer.h"

// -----

unsigned short Serializer::calculateMaximumLineWidth() {

    // IMPORTANT NOTICE: In this step, We need to calculate the bigger size of 
    // one line in given array. When we declare a 2D array, program will allocate 
    // each line of this array with same line width. Because of this, 
    // for avoiding of overflow, we must calculate the upper bound all given line

    // Store the size of received data at the here
    unsigned short sizeofBuffer = strlen(decodedList.givenData);

    char *bufferData = (char *) malloc(sizeof (char) * (sizeofBuffer + 1));
    strcpy(bufferData, decodedList.givenData);
    bufferData[sizeofBuffer] = '\0';

    // -----

    // Store the size of received data at the here
    unsigned short maximumLineWidth = 0;

    // Store splitting data at the here
    char *tokenizer = strtok(bufferData, decodedList.delimiter);

    while (tokenizer != NULL) {

        // Store the size of received data at the here
        unsigned short sizeofTokenizer = strlen(tokenizer);

        if (sizeofTokenizer > maximumLineWidth)
            maximumLineWidth = sizeofTokenizer;

        tokenizer = strtok(NULL, decodedList.delimiter);
    }

    // Do not forget, strictly free up memory
    free(bufferData);

    return maximumLineWidth;
}

void Serializer::clearDecodedList(bool isAllData) {

    // IMPORTANT NOTICE: seems so confused, right? 
    // When you free up a pointer, you can not use it again anymore 
    // But when you make reassigning NULL to a pointer after the freeing up, 
    // You can this pointer again very well

    free(decodedList.delimiter);
    decodedList.delimiter = NULL;

    free(decodedList.givenData);
    decodedList.givenData = NULL;

    if (isAllData) {

        for (unsigned short index = 0; index < decodedList.sizeofResultData; index++) {
            free(decodedList.resultData[index]);
            decodedList.resultData[index] = NULL;
        }

        free(decodedList.resultData);
        decodedList.resultData = NULL;
    }

    // -----

    decodedList.sizeofDelimiter = 0;
    decodedList.sizeofGivenData = 0;
    decodedList.sizeofResultData = 0;
}

void Serializer::clearEncodedList(bool isAllData) {

    // IMPORTANT NOTICE: seems so confused, right? 
    // When you free up a pointer, you can not use it again anymore 
    // But when you make reassigning NULL to a pointer after the freeing up, 
    // You can this pointer again very well

    free(encodedList.delimiter);
    encodedList.delimiter = NULL;

    for (unsigned short index = 0; index < encodedList.sizeofGivenData; index++) {
        free(encodedList.givenData[index]);
        encodedList.givenData[index] = NULL;
    }

    free(encodedList.givenData);
    encodedList.givenData = NULL;

    if (isAllData) {
        free(encodedList.resultData);
        encodedList.resultData = NULL;
    }

    // -----

    encodedList.sizeofDelimiter = 0;
    encodedList.sizeofResultData = 0;
    encodedList.sizeofGivenData = 0;
}

void Serializer::fillDecodedList() {

    // Store the size of received data at the here
    unsigned short maximumLineWidth = calculateMaximumLineWidth();

    // Store splitting data at the here
    char *tokenizer = strtok(decodedList.givenData, decodedList.delimiter);

    while (tokenizer != NULL) {

        // Malloc and realloc a sentence,  a list of words
        if (decodedList.resultData == NULL)
            decodedList.resultData = (char **) malloc(sizeof (char *) * (++decodedList.sizeofResultData));
        else
            decodedList.resultData = (char **) realloc(decodedList.resultData, sizeof (char *) * (++decodedList.sizeofResultData));

        // Malloc and realloc a word, a list of characters, after carry it
        decodedList.resultData[decodedList.sizeofResultData - 1] = (char *) malloc(sizeof (char) * (maximumLineWidth + 1));
        strcpy(decodedList.resultData[decodedList.sizeofResultData - 1], tokenizer);
        decodedList.resultData[decodedList.sizeofResultData - 1][maximumLineWidth] = '\0';

        tokenizer = strtok(NULL, decodedList.delimiter);
    }

    // Do not forget, strictly free up memory
    free(tokenizer);

    // IMPORTANT NOTICE: Probably it is looking unnecessary ...
    // But it has a very important role on memory. With this code,
    // Program will resize the array and put a NULL parameter to tail
    decodedList.resultData = (char **) realloc(decodedList.resultData, sizeof (char *) * (decodedList.sizeofResultData + 1));
    decodedList.resultData[decodedList.sizeofResultData] = '\0';
}

void Serializer::fillEncodedList() {

    // Store the size of received data at the here
    int sizeofAbsolute = encodedList.sizeofDelimiter - encodedList.sizeofGivenData;

    // -----

    // Increase total count of data size
    encodedList.sizeofResultData += encodedList.sizeofDelimiter;
    encodedList.resultData = (char *) malloc(sizeof (char) * (encodedList.sizeofResultData + 1));

    // Declare an variable for storing done separator
    unsigned short checkedDelimiter = 0;
    unsigned short currentIndex = 0;

    // If absolute value is bigger than 0, add a delimiter to the first index
    if (sizeofAbsolute >= 0)
        encodedList.resultData[currentIndex++] = encodedList.delimiter[checkedDelimiter++];

    for (unsigned short array = 0; array < encodedList.sizeofGivenData; array++) {

        // Store the size of received data at the here
        unsigned short sizeofBuffer = strlen(encodedList.givenData[array]);

        // Get line by line characters and fill result data
        for (unsigned short index = 0; index < sizeofBuffer; index++)
            encodedList.resultData[currentIndex++] = encodedList.givenData[array][index];

        if (checkedDelimiter != encodedList.sizeofDelimiter)
            encodedList.resultData[currentIndex++] = encodedList.delimiter[checkedDelimiter++];
    }

    // Finally add end null character to tail
    encodedList.resultData[currentIndex++] = '\0';
}

bool Serializer::decodeData() {

    // Declare an variable for storing done separator
    unsigned short checkedDelimiter = 0;

    for (unsigned short index = 0; index < decodedList.sizeofGivenData; index++) {

        // Found status flag, using for to find operate
        bool foundFlag = false;

        for (unsigned short subIndex = 0; subIndex < decodedList.sizeofDelimiter; subIndex++) {
            if (decodedList.delimiter[subIndex] == decodedList.givenData[index]) {
                foundFlag = true;
                break;
            }
        }

        // Is a delimiter not found in delimiter list, jump to next
        if (!foundFlag)
            continue;

        if (checkedDelimiter >= decodedList.sizeofDelimiter)
            return false;

        if (decodedList.delimiter[checkedDelimiter++] != decodedList.givenData[index])
            return false;
    }

    if (checkedDelimiter < decodedList.sizeofDelimiter)
        return false;

    // Arrived final function
    fillDecodedList();

    return true;
}

bool Serializer::encodeData() {

    // IMPORTANT NOTICE: The absolute value always must be 0 or zero
    // For example, If size of given data is bigger or smaller than 
    // the size of delimiters, we can not have enough delimiters for encoding
    // For this reason, When ABS(s) of delimiters and data is 0 or 1,
    // encoding can be performed very well
    if (abs(encodedList.sizeofGivenData - encodedList.sizeofDelimiter) > 1)
        return false;

    // -----

    for (unsigned short array = 0; array < encodedList.sizeofGivenData; array++) {

        // Store the size of received data at the here
        unsigned short sizeofBuffer = strlen(encodedList.givenData[array]);

        // Check that whether given data includes a delimiters or not
        for (unsigned short index = 0; index < sizeofBuffer; index++)
            for (unsigned short iterator = 0; iterator < encodedList.sizeofDelimiter; iterator++)
                if (encodedList.givenData[array][index] == encodedList.delimiter[iterator])
                    return false;

        // Increase total count of data size
        encodedList.sizeofResultData += sizeofBuffer;
    }

    // Arrived final function
    fillEncodedList();

    return true;
}

// -----

/**
 * Constructors 
 * 
 * @param -
 * @return -
 */
Serializer::Serializer() {
}

// -----

char **Serializer::decode(unsigned short sizeofDelimiter, char delimiter[], unsigned short sizeofGivenData, char givenData[]) {

    // Clear last stored data
    clearDecodedList(true);

    if (delimiter == NULL)
        return NULL;

    if (givenData == NULL)
        return NULL;

    if (sizeofDelimiter == 0)
        return NULL;

    // Store all received data size on lib
    decodedList.sizeofDelimiter = sizeofDelimiter;
    decodedList.sizeofGivenData = sizeofGivenData;

    // -----

    decodedList.delimiter = (char *) malloc(sizeof (char) * (sizeofDelimiter + 1));
    strcpy(decodedList.delimiter, delimiter);
    decodedList.delimiter[sizeofDelimiter] = '\0';

    decodedList.givenData = (char *) malloc(sizeof (char) * (sizeofGivenData + 1));
    strcpy(decodedList.givenData, givenData);
    decodedList.givenData[sizeofGivenData] = '\0';

    // -----

    bool decodeDataFlag = decodeData();

    if (!decodeDataFlag)
        clearDecodedList(true);
    else
        clearDecodedList(false);

    return (decodeDataFlag ? decodedList.resultData : NULL);
}

char *Serializer::encode(unsigned short sizeofDelimiter, char delimiter[], unsigned short sizeofGivenData, char *givenData[]) {

    // Clear last stored data
    clearEncodedList(true);

    if (delimiter == NULL)
        return NULL;

    if (givenData == NULL)
        return NULL;

    if (sizeofDelimiter == 0)
        return NULL;

    // Store all received data size on lib
    encodedList.sizeofDelimiter = sizeofDelimiter;
    encodedList.sizeofGivenData = sizeofGivenData;

    // -----

    encodedList.delimiter = (char *) malloc(sizeof (char) * (sizeofDelimiter + 1));
    strcpy(encodedList.delimiter, delimiter);
    encodedList.delimiter[sizeofDelimiter] = '\0';

    // IMPORTANT NOTICE: In Clang, there is no way out to copy 2D array
    // If you want to do that, first you must allocate main array memory
    // After that you must to fill each line of main array
    encodedList.givenData = (char **) malloc(sizeof (char *) * (sizeofGivenData + 1));
    for (unsigned short index = 0; index < sizeofGivenData; index++) {
        // Store the size of received data at the here
        unsigned short sizeofArray = strlen(givenData[index]);
        encodedList.givenData[index] = (char *) malloc(sizeof (char) * (sizeofArray + 1));
        strcpy(encodedList.givenData[index], givenData[index]);
    }
    encodedList.givenData[sizeofGivenData] = '\0';

    bool encodeDataFlag = encodeData();

    if (!encodeDataFlag)
        clearEncodedList(true);
    else
        clearEncodedList(false);

    return (encodeDataFlag ? encodedList.resultData : NULL);
}

// -----

/**
 * Preinstantiate Objects
 * 
 * @param -
 * @return -
 */
Serializer Serialization = Serializer();