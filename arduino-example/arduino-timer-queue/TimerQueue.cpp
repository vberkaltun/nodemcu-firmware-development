/*
 * TimerQueue.cpp
 * TIMER QUEUE - 29.03.2018
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

#include "TimerQueue.h"

bool Timer::isRegistered(void (*pointer)(void)) {

    bool isRegisteredFlag = false;

    if (timerQueueArray != NULL) {
        for (byte currentQueueCount = 0; currentQueueCount < timerQueueCount; currentQueueCount++) {
            if (pointer == timerQueueArray[currentQueueCount].pointer) {

                isRegisteredFlag = true;
                break;
            }
        }
    }

    return isRegisteredFlag;
}

bool Timer::checkRange(uint16_t _intervalMillis) {

    if (_intervalMillis < DEFAULT_INTERVAL_MILLIS_MIN)
        return false;

    if (_intervalMillis > DEFAULT_INTERVAL_MILLIS_MAX)
        return false;

    return true;
}

// -----

/**
 * Constructors 
 * 
 * @param -
 * @return -
 */
Timer::Timer() {

}

// -----

bool Timer::enqueue(void (*pointer)(void)) {

    return this->enqueue(pointer, DEFAULT_INTERVAL_MILLIS, DEFAULT_STATUS);
}

bool Timer::enqueue(void (*pointer)(void), uint16_t _intervalMillis) {

    return this->enqueue(pointer, _intervalMillis, DEFAULT_STATUS);
}

bool Timer::enqueue(void (*pointer)(void), bool _status) {

    return this->enqueue(pointer, DEFAULT_INTERVAL_MILLIS, _status);
}

bool Timer::enqueue(void (*pointer)(void), uint16_t _intervalMillis, bool _enabledStatus) {

    bool enqueueFlag = false;

    if (!isRegistered(pointer) && checkRange(_intervalMillis) && timerQueueCount < DEFAULT_QUEUE_SIZE) {

        if (timerQueueArray == NULL)
            timerQueueArray = (data *) malloc(sizeof (data)*(++timerQueueCount));
        else
            timerQueueArray = (data *) realloc(timerQueueArray, sizeof (data)*(++timerQueueCount));

        timerQueueArray[timerQueueCount - 1].pointer = pointer;
        timerQueueArray[timerQueueCount - 1].intervalMillis = _intervalMillis;
        timerQueueArray[timerQueueCount - 1].previousMillis = millis();
        timerQueueArray[timerQueueCount - 1].enabledStatus = _enabledStatus;

        enqueueFlag = true;
    }

    return enqueueFlag;
}

bool Timer::dequeue(void (*pointer)(void)) {

    bool dequeueFlag = false;

    if (isRegistered(pointer)) {

        for (byte currentQueueCount = 0; currentQueueCount < timerQueueCount; currentQueueCount++) {

            if (timerQueueArray[currentQueueCount].pointer == pointer) {

                if (timerQueueCount == 1) {
                    timerQueueArray = NULL;
                    timerQueueCount = 0;
                } else {
                    timerQueueArray[currentQueueCount] = timerQueueArray[timerQueueCount - 1];
                    timerQueueArray = (data *) realloc(timerQueueArray, sizeof (data)*(--timerQueueCount));
                }

                dequeueFlag = true;
                break;
            }
        }
    }

    return dequeueFlag;
}

// -----

void Timer::startTimer() {

    for (byte currentQueueCount = 0; currentQueueCount < timerQueueCount; currentQueueCount++)
        timerQueueArray[currentQueueCount].previousMillis = millis();

    timerEnabledStatus = true;
}

void Timer::loopTimer() {

    while (timerEnabledStatus && timerQueueArray != NULL) {

        for (byte currentQueueCount = 0; currentQueueCount < timerQueueCount; currentQueueCount++) {
            if (timerQueueArray[currentQueueCount].enabledStatus == true) {

                uint16_t currentMillis = millis();

                if (currentMillis - timerQueueArray[currentQueueCount].previousMillis >= timerQueueArray[currentQueueCount].intervalMillis) {

                    // save the last time you blinked the LED
                    timerQueueArray[currentQueueCount].previousMillis = currentMillis;
                    timerQueueArray[currentQueueCount].pointer();
                }
            }
        }
    }
}

void Timer::stopTimer() {

    timerEnabledStatus = false;
}

// -----

bool Timer::startProcess(void (*pointer)(void)) {

    bool startProcessFlag = false;

    for (byte currentQueueCount = 0; currentQueueCount < timerQueueCount; currentQueueCount++) {

        // save the last time you blinked the LED
        if (timerQueueArray[currentQueueCount].pointer == pointer) {
            timerQueueArray[currentQueueCount].previousMillis = millis();
            timerQueueArray[currentQueueCount].enabledStatus = true;

            startProcessFlag = true;
            break;
        }
    }

    return startProcessFlag;
}

bool Timer::stopProcess(void (*pointer)(void)) {

    bool stopProcessFlag = false;

    for (byte currentQueueCount = 0; currentQueueCount < timerQueueCount; currentQueueCount++) {

        // save the last time you blinked the LED
        if (timerQueueArray[currentQueueCount].pointer == pointer) {
            timerQueueArray[currentQueueCount].enabledStatus = false;

            stopProcessFlag = true;
            break;
        }
    }

    return stopProcessFlag;
}

// -----

/**
 * Preinstantiate Objects
 * 
 * @param -
 * @return -
 */
Timer TimerQueue = Timer();
