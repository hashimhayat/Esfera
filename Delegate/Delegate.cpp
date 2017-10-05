//
// Created by Hashim Hayat on 4/25/17.
//

#include "Delegate.h"


Delegate::Delegate(int size, std::string directoryPath) {

    cache_size = size;
    DirPath = directoryPath;

    cache_memory = new Cache(cache_size);
    file_manager = new FileManager(directoryPath);
    std::cout << "LRU Cache (" + std::to_string(cache_size) + ") and File Manager Initialized" << std::endl;

}

/*
    Insert the new key value pair into cache and file.
*/

int Delegate::insert(std::string key, std::string value) {

    if (cache_memory->insert(key, value) == 1 && file_manager->insert(key, value) == 1){
        return 1;
    }
    return 0;
}

/*
    First look up in cache. If its present then return value.
    If not in cache then add to cache and check the file.
*/


int Delegate::lookup(std::string key, std::string &value) {

    std::string retVal;
    if (cache_memory->lookup(key, retVal) == 1) {
        value = retVal;
        std::cout << "Cache Hit" << std::endl;
        return 1;
    } else if (file_manager->fileExists(key) == 1) {
        file_manager->lookup(key, retVal);
        value = retVal;
        cache_memory->insert(key, value);
        std::cout << "Not in Cache" << std::endl;
        return 1;
    }
    return 0;
}

/*
    Remove from the cache and file.
*/


int Delegate::remove(std::string key) {

    if (cache_memory->remove(key) == 1 && file_manager->remove(key) == 1){
        return 1;
    }

    return 0;
}

Cache::Cache(const int size) {
    cache_size = size;
    cache_memory = ThreadSafeKVStore<std::string, pair>(false);
}

/*
    Inserts an Item into the Cache Memory
    Return 1 for success and 0 for failure.
 */


int Cache::insert(std::string key, std::string value) {

    if (cache_memory.size() < cache_size){

        pair p; p.value = value; p.usage += 1;
        cache_memory.add(key, p);
        return 1;

    } else {
        performLRU();
        return insert(key, value);
    }

    return 0;
}

/*
    Removes the Least Recently Used element from the cache memory
    Returns 1 for success.
*/

int Cache::performLRU() {

    int leastUsage = 32767;
    std::string LRU_KEY;

    typename std::unordered_map<std::string, pair>::iterator begin;
    typename std::unordered_map<std::string, pair>::iterator end;

    cache_memory.getIterators(begin, end);

    for ( auto it = begin; it!= end; ++it ){

        std::string key = it->first;
        pair p = it->second;
        if (p.usage < leastUsage){
            leastUsage = p.usage;
            LRU_KEY = key;
        }
    }

    if (cache_memory.remove(LRU_KEY) == 0){
        return 1;
    }

    return 0;
}

/*
    Checks if a key value pair is available in the cache memory
    return 1 and value if present, otherwise 0
*/

int Cache::lookup(std::string key, std::string &value) {

    pair retVal;
    if (cache_memory.lookup(key, retVal) == 0) {
        retVal.usage++;
        value = retVal.value;
        return 1;
    }

    return 0;
}

/*
    Removes a key value pair from the cache memory
    Returns 1 for success (key removed or was not present) and 0 for failure.
*/

int Cache::remove(std::string key) {

    if (cache_memory.remove(key) == -1){
        return 0;
    }

    return 1;
}

FileManager::FileManager(const std::string path) {
    PATH = path;
    std::string command = "exec rm -r " + path + "/*";
    system(command.c_str());
    directory = new ThreadSafeKVStore<std::string, std::string>(false);
}

/*
    Creates a new file.
    Gives the file the name specific by the key.
    Writes the value into the file.
    Stores the file into the directory
    Return 1 for success and 0 for failure
*/

int FileManager::createFile(const std::string key, const std::string value) {

    pthread_mutex_lock( &file_mutex );
    int ret = 0;
    std::string path = PATH + "/" + key + ".txt";
    std::ofstream newFile (path);
    if (newFile.is_open()) {
        newFile << value;
        newFile.close();
        ret = 1;
    }
    pthread_mutex_unlock( &file_mutex );
    return ret;
}

/*
    Reads a file and reports its value and return 1
    If the file does not exist or any other issue happens return 0
*/

int FileManager::readFile(const std::string key, std::string &value) {

    int ret = 0;
    std::string path = PATH + "/" + key + ".txt";
    if (fileExists(key)){
        std::string line;
        std::ifstream file (path);
        if (file.is_open()) {
            while ( getline (file, line) ){
                value += line;
            }
            file.close();
        }
        ret = 1;
    }
    return ret;
}

/*
    Writes to a file that already exists.
 */

int FileManager::writeFile(const std::string key, const std::string value) {

    int ret = 0;
    std::string path = PATH + "/" + key + ".txt";
    if (fileExists(key)){
        std::ofstream afile (path);
        if (afile.is_open()) {
            afile << value;
            afile.close();
            ret = 1;
        }
    }
    return ret;
}

/*
    Deletes a file that already exists
*/

int FileManager::deleteFile(const std::string key) {

    int ret = 0;
    std::string path = PATH + "/" + key + ".txt";
    if (fileExists(key)){
        if (std::remove(path.c_str()) == 0){
            ret = 1;
        }
    }
    return ret;
}

/*
    Returns 1 if the file exists, 0 if it does not exists
*/

int FileManager::fileExists(const std::string key) {

    pthread_mutex_lock( &file_mutex );
    int ret = 0;
    std::string path = PATH + "/" + key + ".txt";
    std::ifstream f(path.c_str());
    if (f.good()) {
        ret = 1;
    }
    pthread_mutex_unlock( &file_mutex );
    return ret;
}

int FileManager::insert(std::string key, std::string value) {

    if (fileExists(key) == 1){
        return writeFile(key, value);
    } else {
        return createFile(key, value);
    }
    return 0;
}

int FileManager::lookup(std::string key, std::string &value) {
    return readFile(key, value);
}

int FileManager::remove(std::string key) {
    return deleteFile(key);
}
















