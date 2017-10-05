//
// Created by Hashim Hayat on 3/25/17.
//

#include "ThreadPool.h"

/*
    Constructor
    Init the Ready Thread Queue
    Takes N, the number of threads to be spawned.
    Takes the KVStore
    Binds the KV Store to the Thread Pool
*/

ThreadPoolServer::ThreadPoolServer(int N, int cache_size, std::string storage_path) {
    spawnThreads(N);
    delegate = new Delegate(cache_size, storage_path);
}

/*
    Binds the a KVStore to the Thread Pool
*/

void ThreadPoolServer::bindToStore(ThreadSafeKVStore<std::string, std::string> *store) {
    local_KVStore = store;
}

/*
    Spawns n threads and enqueue them into the Ready Thread Queue
    Return 1 if success else return 0
*/

int ThreadPoolServer::spawnThreads(int N) {

    // Creating an array of N Threads IDs
    NUM_THREADS = N;
    pthread_t tids[NUM_THREADS];

    int rc;
    for (int t = 0; t < NUM_THREADS; ++t){

        pthread_attr_t attr;
        pthread_attr_init(&attr);
        rc = pthread_create(&tids[t], &attr, &do_action, this);

        if (rc){
            std::cout << "Error:unable to create thread, " << rc << std::endl;
            exit(EXIT_FAILURE);
        } else {
            // DEBUG
        }
    }
    return 0;
}


int ThreadPoolServer:: handleClientRequest(req_token req){

    /* Accepting Client Request */

    std::cout << "Serving a Request From: " << req.clt_fd << std::endl;

    int req_id = req.id;
    int client_fd = req.clt_fd;

    HTTPReq request(client_fd);

    if (0 != request.parse() || request.isMalformed()) {
        std::cout << "Failed to parse sample request" << std::endl;
        return -1;
    }

    std::string method = request.getMethod();
    std::string path = request.getURI();
    std::string body = request.getBody();
    std::string connection = request.getConnection();

    std::cout << request << std::endl;

    /* Manipulating the KV STORE */

    /*
        GET /key HTTP/1.1:
        Search the K-V store for key, and return it in a 200 OK
        response if it exists. Return a 404 Not Found response if it does not exist.
    */

    if (method == "GET") {

        if (path == "/LOGS"){

            stats.update_logs("LOGS");
            stats.print_logs(client_fd);

        } else {
            std::string resp_val;
            int ret;
            ret = delegate->lookup(path, resp_val);

            // Updating logs for LOOKUP
            stats.update_logs("LOOKUP");

            // Building Response
            if (ret == 1) {

                HTTPResp response(200, resp_val, false);

                if (!response.isMalformed()) {
                    send(client_fd, response.getResponse().c_str(), (int) strlen(response.getResponse().c_str()), 0);
                }

            } else {
                HTTPResp response(404, "Not Found: " + path, true);

                if (!response.isMalformed()) {
                    send(client_fd, response.getResponse().c_str(), (int) strlen(response.getResponse().c_str()), 0);
                }
            }
        }

    }

        /*
            POST /key HTTP/1.1:
            Insert the key with the value found in the POST request body.
            Return a 200 OK; an empty response body is acceptable. If the key already
            exists, replace it, and still return 200 OK. To make this process more computationally
            expensive, also compute the MD5 hash of the value and store it, either in the same
            appropriately typed ThreadSafeKVStore as the value itself, or in a separate store. Be
            aware of concurrency concerns, including whatever invariants you define for the
            globally visible key-value pair and the value’s globally visible MD5 hash. Note that you
            need not provide a way to get this MD5 hash out again in this phase of the lab.
        */

    else if (method == "POST") {

        // String to Integer
        std::string value = body.c_str();

        int ret;
        ret = delegate->insert(path, value);

        if (ret == 1) {

            // Updating logs for INSERT
            stats.update_logs("INSERT");

            HTTPResp response(200, "Value Added: " + path, false);

            if (!response.isMalformed()) {
                // Send response to the client
                send(client_fd, response.getResponse().c_str(), (int) strlen(response.getResponse().c_str()), 0);
            }
        } else {

            HTTPResp response(200, "Value Not Added: " + path, false);

            if (!response.isMalformed()) {
                // Send response to the client
                send(client_fd, response.getResponse().c_str(), (int) strlen(response.getResponse().c_str()), 0);
            }

        }
    }

        /*
            DELETE /key HTTP/1.1: Delete the key from the K-V store. Return 200 OK if it was
            there, or 404 Not Found if it did not exist.
        */

    else if (method == "DELETE") {

        int ret;
        ret = delegate->remove(path);

        // Building Response

        if (ret == 1) {

            // Updating logs for DELETE
            stats.update_logs("DELETE");

            HTTPResp response(200, "Deleted: " + path, false);

            if (!response.isMalformed()) {
                // Send response to the client
                send(client_fd, response.getResponse().c_str(), (int) strlen(response.getResponse().c_str()), 0);
            }

        } else {
            HTTPResp response(404, "Not Found: " + path, true);

            if (!response.isMalformed()) {
                // Send response to the client
                send(client_fd, response.getResponse().c_str(), (int) strlen(response.getResponse().c_str()), 0);
            }
        }

    } else {
        std::cout << "Unsupported Method!" << std::endl;
    }

    stats.end_timer(req_id);
    close(client_fd);
    return 0;

}

/* Establish a Connection and Start Listening for Client Requests */

int ThreadPoolServer::establishConnection() {

    int PORT = 8001;

    int server_fd, client_fd, err, ret = 0;
    struct sockaddr_in server, client;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0){ std::cout << "Cannot create Socket!" << std::endl; }

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    int opt_val = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);

    err = bind(server_fd, (struct sockaddr *) &server, sizeof(server));
    if (err < 0) { std::cout << "Cannot bind socket!" << std::endl; ret = err; }

    err = listen(server_fd, 128);
    if (err < 0) { std::cout << "Cannot Listen on socket!" << std::endl; ret = err; }

    printf("ThreadPool Server is listening on %d with %d threads\n", PORT, NUM_THREADS);


    while (1) {

        socklen_t client_len = sizeof(client);

        client_fd = accept(server_fd, (struct sockaddr *) &client, &client_len);

        if (client_fd < 0) {
            std::cout << "Could not establish new connection!." << std::endl;
            ret = err;
        } else {

            // Handles the Incoming Requests
            int req_id = stats.start_timer();
            // Creating a req token
            req_token req;
            // Assigning request id for sync of start and end times
            req.id = req_id;
            // Assigning Client fd info
            req.clt_fd = client_fd;

            handleIncomingRequest(req);
        }

    }

    return ret;
}

/*
    Work Horse is the function that is taken by each thread.
    Uses the local_KVStore to perform operations according to the requests
    - Threads handle the following:
        - Incoming Connections
        - Read Req
        - Manipulating the ThreadSafeKVStore
        - Send Response
 */

void* ThreadPoolServer::workHorse(void) {

    /* Waiting for Work */

    //std::cout << "Starting thread " << pthread_self() << std::endl;
    while(1) {

        req_token req;
        int ret;
        ret = Requests.listen(req);

        if (ret == 0){
            handleClientRequest(req);
        } else {
            std::cout << "Listening Error" << std::endl;
        }

    }
}

/*
    Handles the Incoming Requests to the Thread Pool Server.

    1. Listens the Requests Queue constantly waiting for a Request
    2. When Request(s) is/are Available:
    3. Check if any threads are available
    4. If Threads are available:
    5. Signal the Thread to perform the Request

    Enqueue the Incoming Request in the Requests queue
    Return 0 (success) Return -1 (error)
*/

int ThreadPoolServer::handleIncomingRequest(req_token req) {

    //Enqueue the Incoming Request
    if (Requests.push(req) == 0) {
        return 0;
    }
    return -1;
}





