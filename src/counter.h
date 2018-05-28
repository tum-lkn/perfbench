#ifndef COUNTER_H
#define COUNTER_H

#include <arpa/inet.h>
#include <sys/time.h>
#include <math.h>
#include <inttypes.h>
#include <string>
#include <iostream>
#include <unistd.h>
#include <queue>
#include <algorithm>
#include <fstream>
#include <numeric>
#include <system_error>
#include <sstream>  
#include <nanomsg/nn.h>
#include <tbb/concurrent_vector.h>
#include <tbb/concurrent_queue.h>
#include <boost/log/trivial.hpp>
#include <vector>
#include <atomic>

#include "utils.h"
#include "testcases.h"

namespace testcases {
    class BaseTestCase;
}

class Counter {    
	double start_time;    
	long long req_total_bytes = 0, resp_total_bytes = 0;
	uint32_t max_time = 0, max_packets = 0;
	
	
	double interval = 1;
	
	int _lds, _loss_threshold;
	std::string _ldsuf;

public:
    testcases::BaseTestCase* testcase;
    
	bool finished = false;
    double interval_start = 0.0;
	
    uint32_t intervals;
	std::atomic<int> interval_requests{ 0 };
	std::atomic<int> interval_responses{ 0 };
	std::atomic<int> interval_losses{ 0 };
    double interval_latency_sum = 0.0;

	std::string report_prefix;

	// Conn-ID to Vector index
	tbb::concurrent_vector<int> conn_ids;

	// Nested Vectors for each connection
	tbb::concurrent_vector<uint32_t> requests;
	tbb::concurrent_vector<uint32_t> responses;
	tbb::concurrent_vector<uint32_t> last_response_xid;
	tbb::concurrent_vector<uint32_t> missing_responses;

    
	// Request XIDS, Timestamps, Pktlen, Msgtypes, Connids
	tbb::concurrent_vector<tbb::concurrent_vector<uint32_t>> req_xids;
	tbb::concurrent_vector<tbb::concurrent_vector<double>> req_times;
	tbb::concurrent_vector<tbb::concurrent_vector<uint32_t>> req_bytes;
	tbb::concurrent_vector<tbb::concurrent_vector<uint32_t>> req_msgtypes;
	
	
	tbb::concurrent_vector<tbb::concurrent_vector<uint32_t>> resp_xids;
	tbb::concurrent_vector<tbb::concurrent_vector<double>> resp_times;
	tbb::concurrent_vector<tbb::concurrent_vector<uint32_t>> resp_bytes;
	tbb::concurrent_vector<tbb::concurrent_vector<uint32_t>> resp_msgtypes;

	Counter();
	~Counter();
	void count_request(uint32_t xid, double time, uint32_t bytes, uint32_t msg_type, size_t conn_index);
	void count_response(uint32_t xid, double time, uint32_t bytes, uint32_t msg_type, size_t conn_index);
	uint8_t conn_id_to_index(int conn_id);

	void newInterval();
	void reset();

	void final_report();
	void finish();
	void set_max_packets(const int max);
	void set_max_time(const int max);
	bool limit_reached();
	Counter& report_interval(double interval);
	double report_interval();
	int livedatasocket();
	Counter& livedatasocket(int socket);
	std::string livedatasuffix();
	Counter& livedatasuffix(std::string suffix);
};

#endif
#pragma once
