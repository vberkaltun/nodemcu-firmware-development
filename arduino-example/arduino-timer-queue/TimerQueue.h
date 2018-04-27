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
        unsigned long previousMillis = 0;
        unsigned long intervalMillis = DEFAULT_INTERVAL_MILLIS;
    } timerQueueArray[DEFAULT_QUEUE_SIZE];

    char timerQueueCount = 0;
    bool timerEnabledStatus = false;

    // -----

    bool isRegistered(void (*pointer)(void));
    bool checkRange(unsigned short intervalMillis);

public:

    Timer();

    bool attach(void (*pointer)(void));
    bool attach(void (*pointer)(void), unsigned short intervalMillis);
    bool attach(void (*pointer)(void), bool enabledStatus);
    bool attach(void (*pointer)(void), unsigned short intervalMillis, bool enabledStatus);

    void start();
    void loop();
    void stop();

    bool startProcess(void (*pointer)(void));
    bool stopProcess(void (*pointer)(void));
};

extern Timer TimerQueue;

#endif