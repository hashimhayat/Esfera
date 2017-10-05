//
// Created by Hashim Hayat on 2/17/17.
//

#ifndef LAB1_THREADSAFEKVSTORE_H
#define LAB1_THREADSAFEKVSTORE_H

#include <unordered_map>
#include <stdlib.h>
#include <string>
#include <pthread.h>
#include <typeinfo>

// Creating the templated ThreadSafeKVStore<K, V> class.

template <typename K, typename V>
class ThreadSafeKVStore {

    private:
        std::unordered_map<K,V> store;

        // MD5 Hash Settings
        bool calc_hash = false;
        std::unordered_map<K,std::string> md5store;

        /* Thread Safety Tools */

        long reader_count = 0;          // # of readers currently reading the store.
        pthread_rwlock_t rwlock;        // Reader - Writer Lock


    public:
        std::unordered_map<K,V> getStore();
        ThreadSafeKVStore(bool hash);
        ThreadSafeKVStore();
        int insert(const K key, const V value);
        int add(const K key, const V value);
        int accumulate(const K key, const V value);
        int lookup(const K key, V& value);
        int remove(const K key);
        int size();
        void getIterators(typename std::unordered_map<K,V>::iterator &B, typename std::unordered_map<K,V>::iterator &E);
};

#endif //LAB1_THREADSAFEKVSTORE_H
