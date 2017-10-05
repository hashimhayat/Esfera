/*
    Created by Hashim Hayat on 3/25/17.
    ::Thread Pool Class::
*/

#ifndef LAB2_THREADPOOL_H
#define LAB2_THREADPOOL_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#include "../HTTP/httpReq.h"
#include "../HTTP/httpResp.h"
#include "../ThreadSafeListenerQueue/TSListenerQueue.cpp"
#include "../Delegate/Delegate.h"

struct req_token {
    int id;
    int clt_fd;
};

class ThreadPoolServer {

    private:
        // Total Threads
        int NUM_THREADS;

        // A Thread Safe Queue of incoming Requests.
        ThreadSafeListenerQueue<req_token> Requests = ThreadSafeListenerQueue<req_token>();

        // Pointer to the KVStore
        ThreadSafeKVStore<std::string, std::string> *local_KVStore;

        // Delegate to manage cache and file management
        Delegate *delegate;

        // Statistics Information
        Statistics stats;

    public:

        ThreadPoolServer(int n, int cache_size, std::string storage_path);
        void bindToStore(ThreadSafeKVStore<std::string, std::string> *store);
        int spawnThreads(int NUM_THREADS);
        int establishConnection();
        void* workHorse(void);
        int handleClientRequest(req_token req);
        int handleIncomingRequest(req_token req);

        static void* do_action(void* arg) {
            return static_cast<ThreadPoolServer*>(arg)->workHorse();
        }
};

#endif //LAB2_THREADPOOL_H











