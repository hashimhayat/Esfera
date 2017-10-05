//
// Created by Hashim Hayat on 4/25/17.
//

#ifndef LAB4_DELEGATE_H
#define LAB4_DELEGATE_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string.h>
#include <cstdlib>
#include "../Performance/Statistics.h"

struct pair {
    std::string value;
    int usage = 0;
};

class FileManager {

private:

    // Path of the working directory
    std::string PATH;

    // A KV store of file names (keys) and file paths
    ThreadSafeKVStore<std::string, std::string> *directory;

    // Mutex to protect file access.
    pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;    // Protects reader_count


public:

    FileManager(const std::string path);
    int createFile(const std::string key, const std::string value);
    int readFile(const std::string key, std::string &value);
    int writeFile(const std::string key, const std::string value);
    int deleteFile(const std::string key);
    int fileExists(const std::string key);
    int insert(std::string key, std::string value);
    int lookup(std::string key, std::string &value);
    int remove(std::string key);

};

class Cache {

private:

    // The size of cache
    int cache_size;

    // The cache memory
    ThreadSafeKVStore<std::string, pair> cache_memory;

public:

    Cache(const int size);
    int insert(std::string key, std::string value);
    int performLRU();
    int lookup(std::string key, std::string &value);
    int remove(std::string key);

};

class Delegate {

    private:

        int cache_size;
        std::string DirPath;

        Cache *cache_memory;
        FileManager *file_manager;

    public:

        Delegate(int cache_size, std::string directory);
        int insert(std::string key , std::string value);
        int lookup(std::string key , std::string &value);
        int remove(std::string key);

};


#endif //LAB4_DELEGATE_H
