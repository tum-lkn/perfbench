#include "reporter.h"
#include "systemstate.h"

int Reporter::report_interval = 1;
bool Reporter::live_report_active = true;
bool Reporter::file_report_active = false;
bool Reporter::finished = false;
std::string Reporter::filepath = "";
std::ofstream Reporter::filestream;

boost::thread Reporter::report_thread;

void Reporter::init(bool no_live_report, std::string interval_report_file) {
	Reporter::live_report_active = !no_live_report;

	if (!interval_report_file.empty()) {
		Reporter::file_report_active = true;
		Reporter::filepath = interval_report_file;
	}

	// Register signal when all tenants connected/finished
	Signals::AllTenantsConnected.connect(boost::bind(&Reporter::start));
	Signals::AllTenantsFinished.connect(boost::bind(&Reporter::finish));
}

void Reporter::start() {
	// Create directory (recursively)
	if(Reporter::file_report_active){
		boost::filesystem::path p(Reporter::filepath);
		boost::filesystem::path dir = p.parent_path();
		if(!dir.string().empty())
			boost::filesystem::create_directories(dir);
	}

	// Start report loop in a thread
	Reporter::report_thread = boost::thread(&Reporter::report_loop);
}

void Reporter::finish() {
	Reporter::finished = true;
}

void Reporter::report_loop() {
	// Initial Sleep
	usleep(Reporter::report_interval * 1000000);

	while(!Reporter::finished){

		if (Reporter::live_report_active) {
			Reporter::live_report();
		}
		if (Reporter::file_report_active) {
			Reporter::file_report();
		}

		// Tell all the counters to start a new interval
		for (uint8_t i = 0; i < SystemState::tenants.size(); i++) {
			SystemState::tenants[i]->testcase->cntr->newInterval();
		}

		usleep(Reporter::report_interval * 1000000);
	}
}

void Reporter::live_report(){
	std::string requests_str;
	std::string responses_str;
	std::string latencies_str;
	std::string loss_str;

	for (uint8_t i = 0; i < SystemState::tenants.size(); i++) {
		Counter* c = SystemState::tenants[i]->testcase->cntr;

		// Loss
		double loss_ratio;
        if (c->interval_requests == 0) {
            loss_ratio = 0.0;
        }
        else {
            if(c->interval_losses < 0){
                loss_ratio = 0.0;
            }
            else{
                loss_ratio = std::min((double)c->interval_losses / (double)c->interval_requests, 1.0);
            }
        }

		// Latency
		double mean_latency;
		if (c->interval_responses == 0) {
			mean_latency = 0.0;
		}
		else {
			mean_latency = (c->interval_latency_sum / c->interval_responses) * 1000;
		}


		double measure_interval = utils::timestamp() - c->interval_start;
		// Reqs and Resps
		//float pps_req = c->interval_requests / (measure_interval);
		//float pps_resp = c->interval_responses / (measure_interval);

		// Assemble our string
		requests_str.append(std::to_string(c->interval_requests));
		requests_str.append(" ");
		responses_str.append(std::to_string(c->interval_responses));
		responses_str.append(" ");
		latencies_str.append(std::to_string(mean_latency));
		latencies_str.append("ms ");
		loss_str.append(std::to_string(loss_ratio));
		loss_str.append(" ");
	}

	printf("\n");

	printf("TX Pkts:\t%s\n", requests_str.c_str());
	printf("RX Pkts:\t%s\n", responses_str.c_str());
	printf("Latency:\t%s\n", latencies_str.c_str());
	printf("Loss:\t%s\n", loss_str.c_str());
	//printf("Latency (mean): %.4fms, ", mean_latency * 1000);
	//printf("Losses: %d, Loss ratio: %.1f%% \n", this->interval_losses, loss_ratio*100.0);
	//printf("\tTotal Requests: %d\n", this->requests);
	//printf("\tTotal Responses: %d\n", this->responses);

}

void Reporter::file_report() {
	std::string requests_str;
	std::string responses_str;
	std::string latencies_str;
	std::string loss_str;

    // Open file (create if not existing)
    Reporter::filestream.open(Reporter::filepath, std::fstream::app);

	for (uint8_t i = 0; i < SystemState::tenants.size(); i++) {
		Counter* c = SystemState::tenants[i]->testcase->cntr;

		// Loss
		double loss_ratio;
		if (c->interval_requests == 0) {
			loss_ratio = 0.0;
		}
		else {
            if(c->interval_losses < 0){
                loss_ratio = 0.0;
            }
            else{
                loss_ratio = std::min((double)c->interval_losses / (double)c->interval_requests, 1.0);
            }
		}

		// Latency
		double mean_latency;
		if (c->interval_responses == 0) {
			mean_latency = 0.0;
		}
		else {
			mean_latency = (c->interval_latency_sum / c->interval_responses) * 1000;
		}


		double measure_interval = utils::timestamp() - c->interval_start;


		// Assemble our string
		requests_str.append(std::to_string(c->interval_requests));
		responses_str.append(std::to_string(c->interval_responses));
		latencies_str.append(std::to_string(mean_latency));
		loss_str.append(std::to_string(loss_ratio));

		if((size_t)i+1 < SystemState::tenants.size()) {
			requests_str.append(",");
			responses_str.append(",");
			latencies_str.append(",");
			loss_str.append(",");
		}
	}

	Reporter::filestream <<
						 requests_str << " "
						 << responses_str << " "
						 << latencies_str << " "
						 << loss_str << "\n";

    Reporter::filestream.close();
}
