//
// Created by Hashim Hayat on 4/11/17.
//

#ifndef LAB3_STATISTICS_H
#define LAB3_STATISTICS_H

#include <pthread.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <random>
#include <float.h>
#include <cmath>
#include <chrono>
#include <list>

#include "../HTTP/httpReq.h"
#include "../HTTP/httpResp.h"
#include "../ThreadSafeKVStore/TSKVStore.cpp"

typedef std::chrono::duration<float> float_seconds;

/*
     Stores start and end time points of each thread.
     There is an id associated with every timer, used for sync.
*/

struct timer {
    int id;
    std::chrono::system_clock::time_point start;
    std::chrono::system_clock::time_point end;
    float_seconds diff;
    float_seconds elapse;
};

class Statistics {

    private:

        // ThreadSafe KVStore to save stats information.
        ThreadSafeKVStore<std::string, int> stats = ThreadSafeKVStore<std::string, int>(false);

        // ThreadSafe KVStore to save elapse time information.
        ThreadSafeKVStore<std::string, float_seconds> elapse_stats = ThreadSafeKVStore<std::string, float_seconds>(false);

        // ThreadSafe KVStore to store timer information for requests <id , time info>
        ThreadSafeKVStore<int, timer> stat_timer = ThreadSafeKVStore<int, timer>(false);

        // Vector that store Request durations
        std::list<float_seconds> elapse_times;

        // Reader - Writer Lock to protect elapse_time list
        pthread_rwlock_t rwlock;

    public:

        Statistics();
        void update_logs(std::string type);
        int print_logs(int client_fd);
        int start_timer();
        void end_timer(int req_id);
        float median(std::list<float_seconds> arr);
        int appendElapse(float_seconds elapse);
};



#endif //LAB3_STATISTICS_H
