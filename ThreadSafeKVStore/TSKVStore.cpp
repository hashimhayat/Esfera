//
// Created by Hashim Hayat on 2/17/17.
//

#include "TSKVStore.h"
#include "../MD5_HASH/md5wrapper.h"

/* Constructor */

template <typename K, typename V>
ThreadSafeKVStore<K,V>::ThreadSafeKVStore(bool hash):
    calc_hash(hash)
{
    pthread_rwlock_init(&rwlock, NULL);
    store.clear();
    md5store.clear();

}

template <typename K, typename V>
ThreadSafeKVStore<K,V>::ThreadSafeKVStore():
    calc_hash(true)
{
    pthread_rwlock_init(&rwlock, NULL);
    store.clear();
    md5store.clear();
}

/*
   * WRITE *
   Should insert the key-value pair if the key doesn’t exist in the hashmap, or update the value if it does.
   Only return -1 if some fatal error occurs (which may not be possible at this point). Return 0 on success.
   Calculates the hash of the value and stores it into md5store
*/
template <typename K, typename V>
int ThreadSafeKVStore<K,V>::insert(const K key, const V value) {

    pthread_rwlock_wrlock(&rwlock);

    // -------- Critical Section Starts --------

    store[key] = value;

    if (calc_hash) {
        // MD5 Hash Generator
        md5wrapper md5;

        //Generating MD5 Hash of the value
        std::string md5hash = md5.getHashFromString(value);
        md5store[key] = md5hash;
    }

    // -------- Critical Section Ends ----------

    pthread_rwlock_unlock(&rwlock);

    return 0;
}

/*
   * WRITE *
   Should insert the key-value pair if the key doesn’t exist in the hashmap, or update the value if it does.
   Only return -1 if some fatal error occurs (which may not be possible at this point). Return 0 on success.
*/

template <typename K, typename V>
int ThreadSafeKVStore<K,V>::add(const K key, const V value) {

    pthread_rwlock_wrlock(&rwlock);

    // -------- Critical Section Starts --------

    store[key] = value;

    // -------- Critical Section Ends ----------

    pthread_rwlock_unlock(&rwlock);

    return 0;
}

/*
    * WRITE *
    Like insert(), but if the key-value pair already exists, accumulate (i.e., add) the new value to the
    existing value. This of course means that the templated V type must support the + operator.
    Because addition on some data types (like strings) is not commutative, make sure that your accumulation
    is in the form stored_value = existing_stored_value + new_value;
    The return values should mirror those for insert().
*/
template <typename K, typename V>
int ThreadSafeKVStore<K,V>::accumulate(const K key, const V value) {

    pthread_rwlock_wrlock(&rwlock);
    // -------- Critical Section Starts --------
    V retVal;
    int ret = -1;

    if (store.end() == store.find(key)){
        store[key] = value;
        ret = 0;
    } else {
        retVal = store[key];
        retVal += value;
        store[key] = retVal;
        ret = 0;
    }
    // -------- Critical Section Ends ---------
    pthread_rwlock_unlock(&rwlock);
    return ret;
}

/*
    * READ *
    Return 0 if the key is present, -1 otherwise. If the key is present, fill the value variable
    (passed by reference) with the associated value.
*/
template <typename K, typename V>
int ThreadSafeKVStore<K,V>::lookup(const K key, V &value) {

    pthread_rwlock_rdlock(&rwlock);
    // -------- Critical Section Starts --------

    int ret;
    if (store.end() == store.find(key)){
        ret = -1;
    } else {
        value = store.at(key);
        ret = 0;
    }

    // -------- Critical Section End -----------
    pthread_rwlock_unlock(&rwlock);

    return ret;
}

/*
    * WRITE *
    Delete the key-value pair with key from the hashmap, if it exists. Do nothing if it does not exist.
    Return 0 on success; return -1 if there is some fatal error. The key not existing is a “success” condition,
    not an error (why? Think about the invariants that the remove() operation implies).
*/
template <typename K, typename V>
int ThreadSafeKVStore<K,V>::remove(const K key) {

    V retVal;
    // If value already exist for the key
    if (this->lookup (key, retVal) == 0){

        pthread_rwlock_wrlock(&rwlock);
        // -------- Critical Section Starts --------
            store.erase(key);
        // -------- Critical Section Ends ----------
        pthread_rwlock_unlock(&rwlock);

        return 0;
    } else {
        return -1;
    }
}

/* Getter for store {unordered_map<K, V>} */
template <typename K, typename V>
std::unordered_map<K, V> ThreadSafeKVStore<K,V>::getStore() {
    pthread_rwlock_wrlock(&rwlock);
    std::unordered_map<K, V> ref = store;
    pthread_rwlock_unlock(&rwlock);
    return ref;
}

/* To get the begin and end iterator of the unordered_map */

template <typename K, typename V>
void ThreadSafeKVStore<K,V>::getIterators(typename std::unordered_map<K,V>::iterator &B, typename std::unordered_map<K,V>::iterator &E){
    pthread_rwlock_wrlock(&rwlock);
    B = store.begin();
    E = store.end();
    pthread_rwlock_unlock(&rwlock);
}

template <typename K, typename V>
int ThreadSafeKVStore<K,V>::size() {
    int size = -1;
    pthread_rwlock_rdlock(&rwlock);
    size = store.size();
    pthread_rwlock_unlock(&rwlock);
    return size;
}










