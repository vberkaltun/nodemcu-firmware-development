/*
 * TimerQueue.h
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

#ifndef TIMERQUEUE_H
#define TIMERQUEUE_H

#include <stdlib.h>

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define DEFAULT_INTERVAL_MILLIS 500
#define DEFAULT_INTERVAL_MILLIS_MIN 10
#define DEFAULT_INTERVAL_MILLIS_MAX 60000
#define DEFAULT_QUEUE_SIZE 32
#define DEFAULT_STATUS true

class Timer {
private:

    struct data {
        void (*pointer)(void) = NULL;
        bool enabledStatus = DEFAULT_STATUS;
        uint16_t previousMillis = 0;
        uint16_t intervalMillis = DEFAULT_INTERVAL_MILLIS;
    };

    struct data *timerQueueArray = NULL;
    byte timerQueueCount = 0;
    bool timerEnabledStatus = false;

    // -----

    bool isRegistered(void (*pointer)(void));
    bool checkRange(uint16_t _intervalMillis);

public:

    Timer();

    bool enqueue(void (*pointer)(void));
    bool enqueue(void (*pointer)(void), uint16_t _intervalMillis);
    bool enqueue(void (*pointer)(void), bool _status);
    bool enqueue(void (*pointer)(void), uint16_t _intervalMillis, bool _enabledStatus);
    bool dequeue(void (*pointer)(void));

    void startTimer();
    void loopTimer();
    void stopTimer();

    bool startProcess(void (*pointer)(void));
    bool stopProcess(void (*pointer)(void));
};

extern Timer TimerQueue;

#endif