#include <iostream>
#include <unistd.h>
#include <random>
#include "ThreadPool/ThreadPool.h"

using namespace std;

int main(int argc, char* argv[]) {

    // Total number of threads
    int NUM_THREADS = 0;
    string directoryPath = "";
    int CACHE_SIZE = 128;

    // Parsing Command-line Input:
    int option;
    while ((option = getopt(argc, argv, "n:p:c:")) != -1) {
        switch (option) {
            case 'n':
                NUM_THREADS = atoi(optarg);
                break;
            case 'p':
                directoryPath = optarg;
                break;
            case 'c':
                CACHE_SIZE = atoi(optarg);
                break;
            default:
                cout << "Format: ./lab1 -n <N> -p </filepath>" << endl;
        }
    }
    cout << "Threads: " << NUM_THREADS << endl;

    // -------------------------------- THREAD STORE SETUP ------------------------------------

    // Initializing the ThreadPool Server with Threads to spawn and KV_Store
    ThreadPoolServer ThreadPool = ThreadPoolServer(NUM_THREADS, CACHE_SIZE, directoryPath);

    // ThreadPool Server and Starts Listening for incoming requests.
    ThreadPool.establishConnection();


    // ----------------------------------------------------------------------------------------

    return 0;
}

