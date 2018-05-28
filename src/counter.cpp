#include "counter.h"

Counter::Counter() {
	this->start_time = 0;
	this->interval_start = 0;
	this->interval_requests = 0;
	this->interval_responses = 0;
	this->interval_losses = 0;
}
Counter::~Counter() {};
void Counter::reset() {
	this->interval_start = 0;
	this->interval_requests = 0;
	this->start_time = utils::timestamp();
	this->intervals = 0;
	std::fill(this->requests.begin(), this->requests.end(), 0);
	std::fill(this->responses.begin(), this->responses.end(), 0);
	this->req_total_bytes = 0;
	this->resp_total_bytes = 0;
	this->finished = false;
}
void Counter::count_request(uint32_t xid, double time, uint32_t bytes, uint32_t msg_type, size_t conn_index) {
	// DEBUG
    //printf("Request: tenant: %d conn_id: %d xid: %d \n",this->testcase->tenant->id, (int)conn_index, xid);


	// Add to request vectors	
	req_xids[conn_index].push_back(xid);
	req_msgtypes[conn_index].push_back(msg_type);
	req_times[conn_index].push_back(time);
	req_bytes[conn_index].push_back(bytes);	
	
	this->req_total_bytes += bytes;
	this->requests[conn_index] = this->requests[conn_index] + 1;
	this->interval_requests++;
}
void Counter::count_response(uint32_t resp_xid, double time, uint32_t bytes, uint32_t msg_type, size_t conn_index) {	
	//DEBUG:
	//printf("Response: tenant: %d conn_id: %d xid: %d\n",this->testcase->tenant->id, (int)conn_index, resp_xid);
    //std::cout << "\tRequests: " << this->requests[conn_index]  << " Responses: " << this->responses[conn_index] << "\n";

    // Flowvisor Flowmod Duplication fix:
    if(resp_xid == this->last_response_xid[conn_index]) return;

    uint32_t req_search_index = std::max((int)(this->responses[conn_index]), 0);
	uint32_t req_search_xid = this->req_xids[conn_index][req_search_index];

    // If we receive a msg with xid < last received xid
	// --> out of order packet
    if(resp_xid < this->last_response_xid[conn_index]){
		//std::cout << "Tenant: " << this->testcase->tenant->id << ", resp_xid < last_xid: " << "Resp ID: " << resp_xid << " Expected ID: " << this->last_response_xid[conn_index] << " .";
		while (resp_xid != req_search_xid) {
			// We go one request --
			req_search_index --;
			if (req_search_index <= 0) {
				// We reached the end of the request table
				return;
			}
			req_search_xid = this->req_xids[conn_index][req_search_index];
		}
		// Found it!
		// DEBUG
		//if(resp_xids[conn_index][req_search_index] == 0)
			//std::cout << "Replacing placeholder with real value\n";
		//std::cout << "Late arrival of a response\n";
		resp_xids[conn_index][req_search_index] = resp_xid;
		resp_msgtypes[conn_index][req_search_index] = msg_type;
		resp_times[conn_index][req_search_index] = time;
		resp_bytes[conn_index][req_search_index] = bytes;

		// Remove the interval loss for this one
		this->interval_losses -= 1;
        this->missing_responses[conn_index] = this->missing_responses[conn_index] - 1;
	}
	// If we receive a msg with xid == last received xid
	// --> duplicate, skip and return
	else if(resp_xid == this->last_response_xid[conn_index]){
		return;
	}
	// If we receive a msg with xid > last received xid
	// MOST COMMON CASE
	else {

        //req_search_index ++;
        //req_search_xid = this->req_xids[conn_index][req_search_index];

        //printf("req_search_index %d\n", req_search_index);
        //printf("req_xids.length %lu\n", this->req_xids[conn_index].size());

		// In case some responses come out of order:
		// i.e. resp xid > expected resp xid
		// Req: 1,2,3,4,5,6,7
		// Resp: 1,2, 6,7, 3,4,5
		//	Search for the corresponding request
		//	Count skipped requests
		if (resp_xid > req_search_xid && resp_xid != 0) {

			//std::cout << "Tenant: " << this->testcase->tenant->id << ", resp_xid > last_xid + 1: " << "Resp ID: " << resp_xid << " Expected ID: " << req_search_xid << " .";
			uint32_t skipped = 0;
			while (resp_xid != req_search_xid) {
				// We move one request further
				skipped++;
				req_search_index ++;
				// We put placeholders for the skipped responses
				// Maybe they will come later

				if (req_search_index > this->requests[conn_index] - 1) {
					// We reached the end of the request table
					// Response
					std::cout << "Corresponding Request not found, skipping... " << "\n";
					return;
				}
				req_search_xid = this->req_xids[conn_index][req_search_index];
			}
			// Found it!
            for(int i=0;i<skipped;++i){
                resp_xids[conn_index].push_back(0);
                resp_msgtypes[conn_index].push_back(0);
                resp_times[conn_index].push_back(0);
                resp_bytes[conn_index].push_back(0);
                this->responses[conn_index] = this->responses[conn_index] + 1;
            }
			this->interval_losses += skipped;
            this->missing_responses[conn_index] = this->missing_responses[conn_index] + skipped;

			//std::cout << "Skipped " << skipped << " Requests" << "\n";
		}

		this->last_response_xid[conn_index] = resp_xid;

		// Add response data
		resp_xids[conn_index].push_back(resp_xid);
		resp_msgtypes[conn_index].push_back(msg_type);
		resp_times[conn_index].push_back(time);
		resp_bytes[conn_index].push_back(bytes);
		this->responses[conn_index] = this->responses[conn_index] + 1;
	}

	// Calc latency
	double latency = time - this->req_times[conn_index][req_search_index];

	// Statistics
	this->resp_total_bytes += bytes;
	this->interval_responses++;
	this->interval_latency_sum += latency;
}

uint8_t Counter::conn_id_to_index(int conn_id) {
	/*printf("Content of conn_ids:\n");
	for ( auto i = this->conn_ids.begin(); i != this->conn_ids.end(); i++ ) {
        std::cout << *i << ",";
    }
	*/

	//printf("Looking for conn_id %d\n", conn_id);
	uint8_t found_index = 0;
	auto it = std::find(this->conn_ids.begin(), this->conn_ids.end(), conn_id);
	if (it == this->conn_ids.end())
	{
	  // Not found
	} 
	else
	{
	  found_index = std::distance(this->conn_ids.begin(), it);
	  //printf("Found it at %d\n", found_index);
	}
	return found_index;
}


void Counter::newInterval() {
	this->interval_start = utils::timestamp();
	this->interval_requests = 0;
	this->interval_responses = 0;
	this->interval_latency_sum = 0;
	this->interval_losses = 0;
	this->intervals++;
}

void Counter::final_report() {
	BOOST_LOG_TRIVIAL(info) << "\n";
	BOOST_LOG_TRIVIAL(info) << "**\tSummary " << this->report_prefix << ":";
	BOOST_LOG_TRIVIAL(info) << "\t" << std::accumulate(this->requests.begin(), this->requests.end(), 0) << " Msgs sent";
	BOOST_LOG_TRIVIAL(info) << "\t" << this->req_total_bytes << " Bytes sent";
	BOOST_LOG_TRIVIAL(info) << "\t" << std::accumulate(this->responses.begin(), this->responses.end(), 0) << " Msgs received";
	BOOST_LOG_TRIVIAL(info) << "\t" << this->resp_total_bytes << " Bytes received";

}
void Counter::finish() {
	this->finished = true;
}
bool Counter::limit_reached() {
	if (this->finished) {
		return true;
	}
	if (this->max_time > 0 && this->start_time > 0) {
		if ((utils::timestamp() - this->start_time) >= this->max_time) {			
			printf("\n**\tTimelimit reached, aborting...\n");
			this->finish();
			return true;
		}
	}
	if (this->max_packets > 0) {
		if ((unsigned)std::accumulate(this->requests.begin(), this->requests.end(), 0) >= this->max_packets) {			
			printf("\n**\tMax packets reached, aborting...\n");
			this->finish();
			return true;
		}
	}
	return false;
}
void Counter::set_max_packets(const int max) {
	this->max_packets = max;
}
void Counter::set_max_time(const int max) {
	this->max_time = max;
}
Counter& Counter::report_interval(double interval) {
	this->interval = interval;
	return *this;
}
double Counter::report_interval() {
	return this->interval;
}
int Counter::livedatasocket() {
	return this->_lds;
}
Counter& Counter::livedatasocket(int socket) {
	this->_lds = socket;
	return *this;
}
std::string Counter::livedatasuffix() {
	return this->_ldsuf;
}
Counter& Counter::livedatasuffix(std::string suffix) {
	this->_ldsuf = suffix;
	return *this;
}
