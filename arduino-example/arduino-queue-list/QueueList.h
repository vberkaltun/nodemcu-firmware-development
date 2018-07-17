/*
 * QueueList.h
 * QUEUE LIST [EXTENDED] - 17.07.2018
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

#ifndef QUEUELIST_H
#define QUEUELIST_H

#ifndef NULL
#define NULL 0
#endif

template <class T>
class QueueList {
private:

    typedef struct node {
        T item;
        node *next, *prev;
    } node;

    int len;
    node *start, *end;

public:
    QueueList();
    ~QueueList();

    void pushBack(const T i);
    void pushFront(const T i);
	
    void popBack();
    void popFront();
	
    T front();
    T back();
	
    int size();
    void clear();
    void clear(unsigned int index);
    T get(unsigned int index);
    T& at(unsigned int index);

    // Array operator
    T& operator[](unsigned int index);
    const T& operator[](unsigned int index) const;

    // Non - critical functions
    int length();
    int indexOf(T val);
};

// Constructor
template <class T>
QueueList<T>::QueueList()
{
    len = 0;
    start = NULL;
    end = NULL;
}

// Destructor
template <class T>
QueueList<T>::~QueueList()
{
    clear();
}

// Push at front
template <class T>
void QueueList<T>::pushFront(const T i)
{
    node* tmp = new node;
    tmp->item = i;
    tmp->next = NULL;
    tmp->prev = NULL;

    if (start == NULL) // If list is empty
    {
        start = tmp;
        end = tmp;
    }
    else // Insert at start
    {
        tmp->next = start;
        start->prev = tmp;
        start = tmp;
    }
    len++; // Increase size counter
}

// Push at back
template <class T>
void QueueList<T>::pushBack(const T i)
{
    node* tmp = new node;
    tmp->item = i;
    tmp->next = NULL;
    tmp->prev = NULL;

    if (end == NULL) // If list is empty
    {
        start = tmp;
        end = tmp;
    }
    else // Insert at the end
    {
        tmp->prev = end;
        end->next = tmp;
        end = tmp;
    }
    len++; // Increase size counter
}

// Pop from front
template <class T>
void QueueList<T>::popFront()
{
    if (start != NULL) {
        node* tmp = start;
        start = start->next;
        if (start != NULL) // Re-link next item to NULL
            start->prev = NULL;
        else // List became empty so we need to clear end
            end = NULL;
        delete tmp;
        len--; // Decrease counter
    }
}

// Pop from back
template <class T>
void QueueList<T>::popBack()
{
    if (end != NULL) {
        node* tmp = end;
        end = end->prev;
        if (end != NULL) //Re-link previous item to NULL
            end->next = NULL;
        else // List became empty so we need to clear start
            start = NULL;
        delete tmp;
        len--; // Decrease counter
    }
}

// Get item from front
template <class T>
T QueueList<T>::front()
{
    if (start != NULL)
        return start->item;
    //TODO: Catch error when list is empty
}

//Get item from back
template <class T>
T QueueList<T>::back()
{
    if (end != NULL)
        return end->item;
    //TODO: Catch error when list is empty
}

// Get size
template <class T>
int QueueList<T>::size()
{
    return this->len;
}

// Clear list
template <class T>
void QueueList<T>::clear()
{
    node* tmp = start;
    while (start != NULL) {
        tmp = start;
        start = start->next;
        delete tmp; // Delete item
        len--; // Decrease counter
    }
    end = NULL;
}
template <class T>
void QueueList<T>::clear(unsigned int index)
{
    node* tmp = start;
    for (int i = 0; i <= index && tmp != NULL; i++) {
        if (i == index) {
            if (tmp->prev != NULL)
                tmp->prev->next = tmp->next;
            else
                start = tmp->next;

            if (tmp->next != NULL)
                tmp->next->prev = tmp->prev;
            else
                end = tmp->prev;

            len--; // Decrease counter
            delete tmp; // Delete item
            break;
        }
        else
            tmp = tmp->next;
    }
}

// Get at index
template <class T>
T QueueList<T>::get(unsigned int index)
{
    node* tmp = start;
    for (int i = 0; i <= index && tmp != NULL; i++) {
        if (i == index)
            return tmp->item;
        else
            tmp = tmp->next;
    }
    //TODO: Catch error when index is out of range
}

template <class T>
T& QueueList<T>::at(unsigned int index)
{
    node* tmp = start;
    for (int i = 0; i <= index && tmp != NULL; i++) {
        if (i == index)
            return tmp->item;
        else
            tmp = tmp->next;
    }
    //TODO: Catch error when index is out of range
}

// Get length
template <class T>
int QueueList<T>::length()
{
    return this->len;
}

// Get index of value
template <class T>
int QueueList<T>::indexOf(T val)
{
    for (int i = 0; i < this->size(); i++)
        if (this->at(i) == val)
            return i;
    return -1;
}

// Array operators
template <class T>
T& QueueList<T>::operator[](unsigned int index)
{
    node* tmp = start;
    for (int i = 0; i <= index && tmp != NULL; i++) {
        if (i == index)
            return tmp->item;
        else
            tmp = tmp->next;
    }
    //TODO: Catch error when index is out of range
}

template <class T>
const T& QueueList<T>::operator[](unsigned int index) const
{
    node* tmp = start;
    for (int i = 0; i <= index && tmp != NULL; i++) {
        if (i == index)
            return tmp->item;
        else
            tmp = tmp->next;
    }
    //TODO: Catch error when index is out of range
}

#endif