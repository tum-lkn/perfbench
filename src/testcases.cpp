#include "testcases.h"

#include "testcases/featuresrequest.cpp"
#include "testcases/echorequest.cpp"
#include "testcases/packetout.cpp"
#include "testcases/statsrequestflow.cpp"
#include "testcases/statsrequestport.cpp"
#include "testcases/packetin.cpp"
#include "testcases/flowmod.cpp"
#include "testcases/flowmodsandwich.cpp"
#include "testcases/flowmodsandwichsingle.cpp"
#include "testcases/packetinsandwich.cpp"
#include "testcases/packetinsandwichsingle.cpp"



using namespace testcases;

// BaseTestCase class
void BaseTestCase::ctrl_event_callback(ControllerEvent* ev) {
	if (ev->get_type() == CTRL_CONN_UP) {

	}
	else if (ev->get_type() == CTRL_CONN_DOWN) {
		this->tenant->tenant_disconnected();
	}
}

void BaseTestCase::ctrl_conn_established(ControllerEvent* ev) {
	// Setup new table entries if ctrl is the sender (ctrl-conn-id is identifier)
	if(!this->switch_sender){
	// Add conn-id to list		
		this->cntr->conn_ids.push_back(ev->ofconn->get_id());

		// Set the list index of the connection (list-index != conn-id)
		size_t i = this->cntr->conn_ids.size() - 1;
		ev->ofconn->list_index = i;

		// Initialize Stats to 0
		this->cntr->requests.push_back(0);
		this->cntr->responses.push_back(0);
		this->cntr->last_response_xid.push_back(0);
		this->cntr->missing_responses.push_back(0);

		// Initialize Req/Resp Lists
		this->cntr->req_xids.push_back(tbb::concurrent_vector<uint32_t>());
		this->cntr->req_times.push_back(tbb::concurrent_vector<double>());
		this->cntr->req_bytes.push_back(tbb::concurrent_vector<uint32_t>());
		this->cntr->req_msgtypes.push_back(tbb::concurrent_vector<uint32_t>());

		this->cntr->resp_xids.push_back(tbb::concurrent_vector<uint32_t>());
		this->cntr->resp_times.push_back(tbb::concurrent_vector<double>());
		this->cntr->resp_bytes.push_back(tbb::concurrent_vector<uint32_t>());
		this->cntr->resp_msgtypes.push_back(tbb::concurrent_vector<uint32_t>());
	}

	// Create OF msg obj
	this->create_packet(this->ofversion);	
}

void BaseTestCase::start() {
	// Setup Counter
	this->cntr = new Counter;
	this->cntr->set_max_packets(this->max_packets());
	this->cntr->set_max_time(this->length());
	this->cntr->livedatasocket(this->livedatasocket());
	this->cntr->livedatasuffix(this->livedatasuffix());
	this->cntr->report_prefix = "Tenant " + std::to_string(this->tenant->id);
	this->cntr->testcase = this;

    // Print out Parameters
	this->log_start();
    
    // The scheduler object computes the burst-size values and inter burst times
    this->scheduler = new Scheduler(this->_bsize_scheduling_distr, this->_btime_scheduling_distr, this->_burst_rate, this->_send_interval, this->_scheduling_file);
    this->next_burst_time = this->scheduler->next_inter_burst_time();

    // Tell the controller to accept connections
	this->start_controller(ip, port, ofversion);

	// Initialize the packet dispatcher
	this->packetdispatcher = new PacketDispatcher;
	this->packetdispatcher->register_testcase(this);

	// Register PacketFactory
	if(this->ofversion == 10)
		this->packetfactory = new PacketFactory10;
	if(this->ofversion == 13)
		this->packetfactory = new PacketFactory13;

	this->start_testcase();
	

}

void BaseTestCase::start_controller(std::string ip, std::string port, int ofVersion) {
	/* Controller Setup */
	// OFController Settings
	int ofsc_version = 4;
	if (ofVersion == 10) {
		ofsc_version = 1;
	}
	OFServerSettings ofsc = OFServerSettings();
	ofsc.supported_version(ofsc_version);
	ofsc.keep_data_ownership(false).echo_interval(600).liveness_check(true).handshake(true).dispatch_all_messages(true);

	// Create the controller	
	ctrl = new Controller(ip.c_str(), std::stoi(port), 1, false);
    ctrl->testcase = this;
	ctrl->set_config(ofsc);
	ctrl->set_nodelay(this->tcpnodelay);

	ctrl->register_for_event(boost::bind(&BaseTestCase::ctrl_event_callback, this, _1), CTRL_CONN_UP);
	ctrl->register_for_event(boost::bind(&BaseTestCase::ctrl_event_callback, this, _1), CTRL_CONN_DOWN);

	// Start the controller
	ctrl->start(false);

	BOOST_LOG_TRIVIAL(info) << "**\t[CP] Controller started at " << ip << ":" << port << " using OF " << ofVersion;
	BOOST_LOG_TRIVIAL(info) << "**\t[CP] Waiting for switch/hv to connect...";


}

void BaseTestCase::log_start() {
	BOOST_LOG_TRIVIAL(info) << "**\tStarted Testcase:"
				 << "\n**\t\tTenant: " << this->tenant->id
				 << "\n**\t\tTestcase: " << this->name
				 << "\n**\t\tListening on: " << this->ip << ":" << this->port
				 << "\n**\t\tOpenFlow Version: " << this->ofversion
				 << "\n**\t\tPPS: " << this->pps()
				 << "\n**\t\tTCP No-Delay: " << this->tcpnodelay
				 << "\n**\t\tLength: " << this->_length
				 << "\n**\t\tInter Burst Time (mean): " << this->send_interval()
				 << "\n**\t\tBurst (mean): " << this->_burst_rate_real
				 << "\n**\t\tIntra Burst Time: " << this->_intra_burst_time
				 << "\n**\t\tBurst Size Distr: " << this->bsize_scheduling_distr()
				 << "\n**\t\tBurst Time Distr: " << this->btime_scheduling_distr()
				 << "\n**\t\tScheduler File: " << this->scheduling_file();
}


void BaseTestCase::start_livedatasocket(std::string address) {
	try {
		// Address
		std::string full_addr = "tcp://" + address;

		// Create the socket
		int sock = nn_socket(AF_SP, NN_PUSH);
		int conn_status = nn_connect(sock, full_addr.c_str());

		if (conn_status >= 0) {
			std::cout << "** Socket connected to " << full_addr << " (Internal ID: " << conn_status << ")\n";
		}
		else {
			printf("** Live Data Connection failed with Error number %d\n", nn_errno());
			printf("** Aborting...\n");
			return;
		}

		this->livedatasocket(sock);
	}
	catch (const std::system_error& err) {
		std::cerr << "Nanomsg Error: " << err.what() << std::endl;
		return;
	}

}

void BaseTestCase::finish_requesting() {
    requesting_finished = true;

    // Tell the tenant object
	this->tenant->testcase_finished();
}


void BaseTestCase::finish() {
	// Stop the controller
	this->ctrl->stop();

	// Summary report
	this->cntr->final_report();

	// Write logfile
	if (this->logging())
		this->write_logfile(logfile);

	// Set myself as finished
	finished = true;	

}

int BaseTestCase::length() {
	return this->_length;
}
BaseTestCase& BaseTestCase::length(const int length) {
	this->_length = length;
	return *this;
}
int BaseTestCase::max_packets() {
	return this->_max_packets;
}
BaseTestCase& BaseTestCase::max_packets(const int max_packets) {
	this->_max_packets = max_packets;
	return *this;
}
int BaseTestCase::pps() {
	return this->_pps;
}
BaseTestCase& BaseTestCase::pps(const int pps) {
	this->_pps = pps;

	// Performance FIX:
	// If SI < 1000/pps  --> set SI to 1000/pps
	if( (float)this->send_interval() < (1000.0/(float)this->pps()) )
		this->send_interval(1000.0/(float)this->pps());
    
	// Burst rate not rounded
	this->_burst_rate_real = this->pps() * this->send_interval() / 1000.0;

	// Burst rate rounded
	this->_burst_rate = floor(this->_burst_rate_real);

	// Missing packets:
	// Because of flooring the actual burst_rate there are missing packets over time
	// Here we calculate that difference
	this->_missing_packets_per_sendinterval = this->_burst_rate_real - this->_burst_rate;

	return *this;
}
double BaseTestCase::send_interval() {
	return this->_send_interval;
}
BaseTestCase& BaseTestCase::send_interval(const double intvl) {
	this->_send_interval = intvl;
	return *this;
}
int BaseTestCase::intra_burst_time() {
	return this->_intra_burst_time;
}
BaseTestCase& BaseTestCase::intra_burst_time(const int ibt) {
	this->_intra_burst_time = ibt;
	return *this;
}
std::string BaseTestCase::bsize_scheduling_distr() {
	return this->_bsize_scheduling_distr;
}
BaseTestCase& BaseTestCase::bsize_scheduling_distr(const std::string distr) {
	this->_bsize_scheduling_distr = distr;
	return *this;
}
std::string BaseTestCase::btime_scheduling_distr() {
	return this->_btime_scheduling_distr;
}
BaseTestCase& BaseTestCase::btime_scheduling_distr(const std::string distr) {
	this->_btime_scheduling_distr = distr;
	return *this;
}
std::string BaseTestCase::scheduling_file() {
	return this->_scheduling_file;
}
BaseTestCase& BaseTestCase::scheduling_file(const std::string f) {
	this->_scheduling_file = f;
	return *this;
}
bool BaseTestCase::logging() {
	return this->_logging;
}
BaseTestCase& BaseTestCase::logging(const bool logging) {
	this->_logging = logging;
	return *this;
}
int BaseTestCase::livedatasocket() {
	return this->_lds;
}
BaseTestCase& BaseTestCase::livedatasocket(int socket) {
	this->_lds = socket;
	return *this;
}
std::string BaseTestCase::livedatasuffix() {
	return this->_ldsuf;
}
BaseTestCase& BaseTestCase::livedatasuffix(std::string suffix) {
	this->_ldsuf = suffix;
	return *this;
}
std::string BaseTestCase::dataplane_ip() {
	return this->_dataplane_ip;
}
BaseTestCase& BaseTestCase::dataplane_ip(std::string ip) {
	this->_dataplane_ip = ip;
	return *this;
}
std::string BaseTestCase::dataplane_mac() {
	return this->_dataplane_mac;
}
BaseTestCase& BaseTestCase::dataplane_mac(std::string mac) {
	this->_dataplane_mac = mac;
	return *this;
}
std::string BaseTestCase::dataplane_intf() {
	return this->_dataplane_intf;
}
BaseTestCase& BaseTestCase::dataplane_intf(std::string intf) {
	this->_dataplane_intf = intf;
	return *this;
}
void BaseTestCase::write_logfile(std::string logfile_path) {
	BOOST_LOG_TRIVIAL(info) << "**\tWriting to: " << logfile_path;

	// Create directory if not existing (recursively)
	boost::filesystem::path p(logfile_path);
	boost::filesystem::path dir = p.parent_path();
	if(!dir.string().empty())
		boost::filesystem::create_directories(dir);

	// Open file (create if not existing)
	std::ofstream f;
	f.open(logfile_path, std::fstream::out);

	for ( uint32_t c_i = 0; c_i < this->cntr->conn_ids.size(); c_i++ ) {
    
		uint32_t skipped = 0;
		for (uint32_t r = 0; r < this->cntr->req_xids.at(c_i).size(); r++)
		{
			uint32_t resp_index = r;

	        uint32_t req_xid = this->cntr->req_xids.at(c_i)[r];
	        uint32_t req_msgtype = this->cntr->req_msgtypes.at(c_i)[r];
	        double req_time = this->cntr->req_times.at(c_i)[r];
	        uint32_t req_bytes = this->cntr->req_bytes.at(c_i)[r];
	        
	        uint32_t resp_msgtype = 0;
	        double resp_time = 0;
	        uint32_t resp_bytes = 0;
	        double lat = 0;

			// Make sure the resp tables have sufficient entries
			if(this->cntr->resp_msgtypes.at(c_i).size() >= resp_index + 1){
				resp_msgtype = this->cntr->resp_msgtypes.at(c_i)[resp_index];
				resp_time = this->cntr->resp_times.at(c_i)[resp_index];
				resp_bytes = this->cntr->resp_bytes.at(c_i)[resp_index];
			}
			if(resp_time == 0){
				lat = 0;
			}
			else {
				lat = (resp_time - req_time) * 1000;
			}



			f << req_xid << " " // XID
				<< req_msgtype << " " // msg_type req
				<< std::fixed
				<< req_time << " " // req_time
				<< req_bytes << " " // req bytes
				<< resp_msgtype << " " // msg_type req
				<< resp_time << " " // req_time
				<< resp_bytes << " " // req bytes
				<< lat << " "  // latency
				<< c_i << "\n"; // conn-id
		}
	}

	f.close();
}
fluid_msg::OFMsg* BaseTestCase::packet() {
	return this->_packet;
}
BaseTestCase& BaseTestCase::packet(fluid_msg::OFMsg* packet) {
	this->_packet = packet;
	return *this;
}

void BaseTestCase::process_error(ErrorEvent* ev) {
	auto msg = new fluid_msg::ErrorCommon(0, 0);
	msg->unpack(ev->data);

	/* TO-DO: PROCESS ERROR / PRINT ERROR */

	//uint32_t xid = msg->xid();
	//double time = utils::timestamp();

	delete msg;

}



/* TestCaseFactory */
BaseTestCase* TestCaseFactory::get_testcase(std::string testcase) {
	if (testcase == "EchoRequest") {
		return new EchoRequest;
	}
	if (testcase == "FeaturesRequest") {
		return new FeaturesRequest;
	}
	if (testcase == "FlowStatsRequest") {
		return new FlowStatsRequest;
	}
	if (testcase == "PortStatsRequest") {
		return new PortStatsRequest;
	}
	if (testcase == "PacketOut") {
		return new PacketOut;
	}
	if (testcase == "PacketIn") {
		return new PacketIn;
	}
    if (testcase == "FlowMod") {
		return new FlowMod;
	}
	if (testcase == "FlowModSandwich") {
		return new FlowModSandwich;
	}
    if (testcase == "FlowModSandwichSingle") {
		return new FlowModSandwichSingle;
	}
	if (testcase == "PacketInSandwich") {
		return new PacketInSandwich;
	}
    if (testcase == "PacketInSandwichSingle") {
		return new PacketInSandwichSingle;
	}
	else {
		std::cout << "Unknown testcase! Using FeaturesRequest..." << "\n";
		return new FeaturesRequest;
	}
}
