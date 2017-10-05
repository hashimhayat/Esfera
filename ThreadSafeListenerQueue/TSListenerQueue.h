//
// Created by Hashim Hayat on 2/17/17.
//

#ifndef LAB1_THREADSAFELISTENERQUEUE_H
#define LAB1_THREADSAFELISTENERQUEUE_H

#include <list>
#include <string>
#include <pthread.h>
#include <iostream>
#include <unordered_map>

template <typename T>
class ThreadSafeListenerQueue {

    private:
        std::list<T> store;

        /* Thread Safety Tools */

        pthread_mutex_t writers = PTHREAD_MUTEX_INITIALIZER;    // Protects projects readers
        pthread_cond_t notEmpty = PTHREAD_COND_INITIALIZER;

    public:
        ThreadSafeListenerQueue();
        int push(const T element);
        int pop(T& element);
        int peek(T& element);
        int listen(T& element);
        bool isEmpty();
};

#endif //LAB1_THREADSAFELISTENERQUEUE_H
