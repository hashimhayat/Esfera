//
// Created by Hashim Hayat on 2/17/17.
//

#include "TSListenerQueue.h"

/* Constructor */

template <typename T>
ThreadSafeListenerQueue<T>::ThreadSafeListenerQueue() {
    store.clear();
}

/*
    * WRITE *
    Pop the least-recently inserted element from the queue and fill in the passed-by-reference
    variable element with its contents, if the queue was not empty. Return 0 if this was successful;
    return 1 if the queue was empty, or -1 if some fatal error occurred.
*/
template <typename T>
int ThreadSafeListenerQueue<T>::pop(T &element) {

    pthread_mutex_lock( &writers );
    // -------- Critical Section Starts --------
    int ret = -1;
    if (!store.empty ()){
        memcpy (&element, &store.front(), sizeof(store.front()));
        store.pop_front();
        ret = 0;
    } else {
        ret = 1;
    }
    // -------- Critical Section Ends ----------
    pthread_mutex_unlock( &writers );
    return ret;
}

/*
    * WRITE *
    Should push the element onto the front of the list, so that it will be the last of
    the items currently on the queue to be removed. Only return -1 if some fatal error
    occurs (which may not be possible at this point). Return 0 on success.
*/
template <typename T>
int ThreadSafeListenerQueue<T>::push(const T element) {

    pthread_mutex_lock( &writers );
    // -------- Critical Section Starts --------

    int ret = -1;
    store.push_back(element);

    if (!store.empty()){
        pthread_cond_signal (&notEmpty);
        ret = 0;
    }
    // -------- Critical Section Ends ----------
    pthread_mutex_unlock( &writers );

    return ret;
}

/*
    * WRITE *
    Similar to pop(), but block until there is an element to be popped.
    Return 0 if an element was returned, or -1 if some fatal error occurred.
*/
template <typename T>
int ThreadSafeListenerQueue<T>::listen(T &element) {

    pthread_mutex_lock ( &writers );
    // -------- Critical Section Starts --------
    int ret = -1;

    while (store.empty ()){
        pthread_cond_wait(&notEmpty, &writers);
    }

    if (!store.empty ()){
        memcpy (&element, &store.front(), sizeof(store.front()));
        store.pop_front();
        ret = 0;
    } else {
        ret = 1;
    }
    // -------- Critical Section Ends ---------
    pthread_mutex_unlock( &writers );

    return ret;
}

/*
    *READ*
    Reads the Front most element of the Queue if it the Queue is not empty.
    Return 0 if an element was returned, or -1 if Queue was empty.
*/

template <typename T>
int ThreadSafeListenerQueue<T>::peek(T& element){

    pthread_mutex_lock ( &writers );
    // -------- Critical Section Starts --------
    int ret = -1;

    if (!store.empty()){
        element = store.front();
        ret = 0;
    }
    // -------- Critical Section Ends ---------
    pthread_mutex_unlock( &writers );

    return ret;
}

template <typename T>
bool ThreadSafeListenerQueue<T>::isEmpty(){

    pthread_mutex_lock ( &writers );
    // -------- Critical Section Starts --------
    bool ret = false;

    if (store.empty()){
        ret = true;
    }
    // -------- Critical Section Ends ---------
    pthread_mutex_unlock( &writers );

    return ret;
}
