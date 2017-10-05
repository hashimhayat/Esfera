//
// Created by Hashim Hayat on 4/11/17.
//

#include "Statistics.h"

Statistics::Statistics() {

    pthread_rwlock_init(&rwlock, NULL);

    stats.accumulate("INSERT",0);
    stats.accumulate("LOOKUP",0);
    stats.accumulate("DELETE",0);
    stats.accumulate("LOGS",0);
    stats.accumulate("TOTAL",0);

    // Keep tracks of first request id - used for Median
    stats.add("FIRST_ID", 0);

    // Keeps track of MIN MAX
    elapse_stats.add("MAX", float_seconds(FLT_MIN));
    elapse_stats.add("MIN", float_seconds(FLT_MAX));


    std::cout << "Performance Analyzer Initialized" << std::endl;
}

// Called as soon as a request arrives
int Statistics::start_timer() {
    timer t;
    t.id = stat_timer.size();
    t.start = std::chrono::system_clock::now();
    stat_timer.add(t.id, t);

    return t.id;
}

// Called when a request is completed
void Statistics::end_timer(int req_id) {

    timer t;
    stat_timer.lookup(req_id,t);
    t.end = std::chrono::system_clock::now();
    t.diff = t.end - t.start;
    t.elapse = std::chrono::duration_cast<float_seconds>(t.diff);
    stat_timer.add(req_id, t);

    // Adding Elapse time to the elapse time list
    appendElapse(t.elapse);

    // SUM of TOTAL REQUEST TIME
    elapse_stats.accumulate("SUM", t.elapse);

    // MAX
    float_seconds MAX;
    elapse_stats.lookup("MAX", MAX);
    if (t.elapse > MAX) {
        elapse_stats.add("MAX", t.elapse);
    }

    // MIN
    float_seconds MIN;
    elapse_stats.lookup("MIN", MIN);
    if (t.elapse < MIN) {
        elapse_stats.add("MIN", t.elapse);
    }

    // MEAN
    int total = 0;
    float_seconds sum;
    stats.lookup("TOTAL", total);
    elapse_stats.lookup("SUM", sum);

    if (total > 0)
        elapse_stats.add("MEAN", sum/total);

    if (total == 1){
        stats.add("FIRST_ID", req_id);
    }

    // MEDIAN
    median(elapse_times);
}

void Statistics::update_logs(std::string type) {
   stats.accumulate(type, 1);
   stats.accumulate("TOTAL", 1);
}

int Statistics::print_logs(int client_fd) {

    int stat_val = 0;
    std::string log = "\nLogs and Statistics: \n------------------------- \n\n";

    stats.lookup("INSERT", stat_val);
    log += "  INSERTS: " + std::to_string(stat_val) + "\n";

    stat_val = 0;
    stats.lookup("LOOKUP", stat_val);
    log += "  LOOKUPS: " + std::to_string(stat_val) + "\n";

    stat_val = 0;
    stats.lookup("DELETE", stat_val);
    log += "  DELETES: " + std::to_string(stat_val) + "\n";

    stat_val = 0;
    stats.lookup("LOGS", stat_val);
    log += "  LOGS: " + std::to_string(stat_val) + "\n";

    stat_val = 0;
    stats.lookup("TOTAL", stat_val);
    log += "  TOTAL: " + std::to_string(stat_val) + "\n\n";

    log += "  Server-Side Stats: \n\n";

    float_seconds stat_val_flt = float_seconds(0.00);
    elapse_stats.lookup("SUM", stat_val_flt);
    log += "  SUM: " + std::to_string(stat_val_flt.count()*1000) + "\n";

    stat_val_flt = float_seconds(0.00);
    elapse_stats.lookup("MEAN", stat_val_flt);
    log += "  MEAN: " + std::to_string(stat_val_flt.count()*1000) + "\n";

    stat_val_flt = float_seconds(0.00);
    elapse_stats.lookup("MEDIAN", stat_val_flt);
    log += "  MEDIAN: " + std::to_string(stat_val_flt.count()*1000) + "\n";

    stat_val_flt = float_seconds(0.00);
    elapse_stats.lookup("MIN", stat_val_flt);
    if (stat_val_flt == float_seconds(FLT_MAX))
        stat_val_flt = float_seconds(0.00);
    log += "  MIN: " + std::to_string(stat_val_flt.count()*1000) + "\n";

    stat_val_flt = float_seconds(0.00);
    elapse_stats.lookup("MAX", stat_val_flt);
    if (stat_val_flt == float_seconds(FLT_MIN))
        stat_val_flt = float_seconds(0.00);
    log += "  MAX: " + std::to_string(stat_val_flt.count()*1000) + "\n";

    // DEBUG
    stats.lookup("DEBUG", stat_val);
    //log += "  DEBUG: " + std::to_string(stat_val) + "\n";


    HTTPResp response(200, log, false);

    if (!response.isMalformed()) {
        send(client_fd, response.getResponse().c_str(), (int) strlen(response.getResponse().c_str()), 0);
        return 1;
    }

    return 0;
}

/*
    Utility Function to calculate median.
 */
float Statistics::median(std::list<float_seconds> alist) {

    // Sorting the list
    alist.sort();

    int first;
    stats.lookup("FIRST_ID", first);

    int diff = int(alist.size()) - first;
    double mid = (diff + 0.0) / 2.0;

    // even
    if (diff % 2 == 0) {

        pthread_rwlock_wrlock(&rwlock);
        int left_i = int(std::floor(mid));
        int right_i = int(std::ceil(mid));

        int i = 0;
        float_seconds sum = float_seconds(0.0);
        for (std::list<float_seconds>::iterator it=alist.begin(); it != alist.end(); ++it) {

            if (i == left_i) {
                sum += *it;
            } else if (i == right_i){
                sum += *it;
                break;
            }
            i++;
        }
        float_seconds avg = sum / 2;
        elapse_stats.add("MEDIAN", avg);
        pthread_rwlock_unlock(&rwlock);


    }

    // odd
    else {

        pthread_rwlock_wrlock(&rwlock);
        int i = 0;
        float_seconds median = float_seconds(0.0);
        for (std::list<float_seconds>::iterator it=alist.begin(); it != alist.end(); ++it) {

            if (i == int(mid)) {
                median = *it;
                break;
            }
            i++;
        }
        elapse_stats.add("MEDIAN", median);
        pthread_rwlock_unlock(&rwlock);
    }




    return 0;
}

int Statistics::appendElapse(float_seconds elapse) {
    pthread_rwlock_wrlock(&rwlock);
    elapse_times.push_back(elapse);
    pthread_rwlock_unlock(&rwlock);
    return 1;
}
