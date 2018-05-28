#include "tenant.h"

#include <boost/log/trivial.hpp>

using namespace testcases;

Tenant::Tenant(std::string ip, std::string port, std::string testcase_s,
	int pps, int send_intvl, int intraBurstTime, int len, int max_packets, int cooldown_time, bool nolog, bool noreport,
	bool livedata, std::string livedata_suffix, int ofVersion, std::string dataplane_ip,
	std::string dataplane_mac, std::string dataplane_intf, std::string logfile, bool tcpnodelay,
	std::string switchIP, int switchPort, std::string sw_type, std::string bsizeschedDistr, std::string btimeschedDistr, std::string schedFile,
	int num_switches)
{

	finished = false;
	connected = false;
	id = SystemState::next_tenant_id();
	switch_type = sw_type;

	/* Testcase Setup */
	// Get testcase object from our factory
	BaseTestCase* testcase = TestCaseFactory::get_testcase(testcase_s);

	// Testcase settings
	testcase->name = testcase_s;	
	testcase->tenant = this;
	testcase->ip = ip;

	// Different port for each tenant
	testcase->port = std::to_string(std::stoi(port) + id - 1);
	testcase->ofversion = ofVersion;
	testcase->length(len);
	testcase->max_packets(max_packets);
	
	testcase->scheduling_file(schedFile);
	if(schedFile.empty()){
		testcase->send_interval(send_intvl);
    	testcase->intra_burst_time(intraBurstTime);
		testcase->pps(pps);	
		testcase->bsize_scheduling_distr(bsizeschedDistr);
		testcase->btime_scheduling_distr(btimeschedDistr);
	}
    
    testcase->cooldown_time = cooldown_time;
	testcase->logging(!nolog);
	testcase->reporting = !noreport;
	testcase->livedatasuffix(livedata_suffix);
	testcase->tcpnodelay = tcpnodelay;
	testcase->switchIP = switchIP;
	testcase->switchPort = switchPort;
	testcase->num_switches = num_switches;

	// Add a suffix to the logfile of each tenant
	std::stringstream _logfile;
	_logfile << logfile << "_" << id << ".csv";
	testcase->logfile = _logfile.str();
	
	testcase->dataplane_ip(dataplane_ip);
	testcase->dataplane_mac(dataplane_mac);
	testcase->dataplane_intf(dataplane_intf);
	testcase->livedatasocket(livedata);

	this->testcase = testcase;
}

Tenant::~Tenant() {}

void Tenant::start() {
	this->testcase->start();
}

void Tenant::tenant_connected() {    
	BOOST_LOG_TRIVIAL(info) << "**\t[CP] Tenant " << this->id << " connected!";
    connected = true;	
	Signals::TenantConnected(this);
}

void Tenant::tenant_disconnected() {
	BOOST_LOG_TRIVIAL(info) << "**\t[CP] Tenant " << this->id << " disconnected!";
    connected = false;
	this->tenant_failed();
}

void Tenant::tenant_failed() {
	Signals::TenantFailed(this);
}

void Tenant::testcase_finished() {
	finished = true;
	SystemState::tenant_finished(this);

}
void Tenant::stop() {
	this->testcase->finish();

}
