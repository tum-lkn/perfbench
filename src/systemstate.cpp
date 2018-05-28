#include "systemstate.h"

bool SystemState::finished = false;
bool SystemState::failed = false;
bool SystemState::running = false;
bool SystemState::switch_started = false;
int SystemState::cooldown_time = 0;
std::vector<Switch*> SystemState::switches;
std::vector<Tenant*> SystemState::tenants;

std::mutex SystemState::sys_mutex;

void SystemState::init() {
	// Register for tenant failure signal
	Signals::TenantFailed.connect(boost::bind(&SystemState::tenant_failed, _1));

	// Thread: Wait for tenants to connect
	boost::thread(boost::bind(&SystemState::wait_for_tenants_to_connect));
}

int SystemState::next_tenant_id() {
	return SystemState::tenants.size() + 1;
}


void SystemState::register_tenant(Tenant* tenant) {
	SystemState::tenants.push_back(tenant);	
}

void SystemState::cooldown() {
	BOOST_LOG_TRIVIAL(info) << "**\tCooldown for " << SystemState::cooldown_time << " seconds...";
    usleep(SystemState::cooldown_time * 1000000);
}

void SystemState::tenant_finished(Tenant* tenant) {
	// When a tenant finishes, check if all the tenants are finished
	// If yes set finished = true
	// If no return
	for (uint8_t i = 0; i < SystemState::tenants.size(); i++) {
		if (!SystemState::tenants[i]->finished)
			return;

	}    
	SystemState::all_tenants_finished();
    
}

void SystemState::all_tenants_finished() {
	Signals::AllTenantsFinished();

	SystemState::cooldown();
	SystemState::stop_tenants();
	SystemState::finished = true;
}

void SystemState::tenant_failed(Tenant* tenant) {
	// When a tenant fails (e.g. connection reset)
	sys_mutex.lock();
	
    // Finish Testcases (e.g. write packet data)
    for (uint8_t i = 0; i < SystemState::tenants.size(); i++) {
		SystemState::tenants[i]->testcase->finish();
	}

		
	SystemState::failed = true;

	sys_mutex.unlock();
}

void SystemState::wait_for_tenants_to_connect() {
    bool _all_t_connected = false;
    while(!_all_t_connected){
        for (uint8_t i = 0; i < SystemState::tenants.size(); i++) {
            if(!SystemState::tenants[i]->connected){
                _all_t_connected = false;
                break;
            }
            else {
                _all_t_connected = true;
            }
        }
        usleep(100000);
    }

	SystemState::all_tenants_connected();
}



void SystemState::all_tenants_connected() {
	BOOST_LOG_TRIVIAL(info) << "**\t[CP] All tenants connected!";

	Signals::AllTenantsConnected();
    
    SystemState::running = true;

    for (uint8_t i = 0; i < SystemState::tenants.size(); i++) {
		// Reset our counter
        SystemState::tenants[i]->testcase->cntr->reset();
		// Start the packetdispatcher send_loop thread
		SystemState::tenants[i]->testcase->send_thread = new boost::thread(&PacketDispatcher::send_loop, SystemState::tenants[i]->testcase->packetdispatcher);
	}	
		
}
Switch* SystemState::start_switch(int id, std::string ip, int port, uint64_t dpid, OFServerSettings ofsc, bool connect) {
	
	// Adding some port names
	std::vector<std::string> ports;
	
	std::ostringstream ss;
	ss << "veth" << (SystemState::switches.size()+1) * 2 - 2;
	std::string intf1 = ss.str();
	ss.str("");
	ss.clear();
	ss << "veth" << (SystemState::switches.size() + 1) * 2 - 1;
	std::string intf2 = ss.str();	


	ports.push_back(intf1);
	ports.push_back(intf2);
	//ports.push_back("eth2");

	BOOST_LOG_TRIVIAL(info) << "**\t[DP] Switch started! DPID: 0x"<< std::hex << dpid << std::dec <<  " Ports: " << intf1 << ", "<< intf2 << " Connecting to: " << ip << ":" << port;

	Switch* sw = new Switch(id, ip, 6633, ports, dpid);

	sw->set_config(ofsc);

	if(connect){
		// Connect switch
		sw->start(false);
	}

	SystemState::switches.push_back(sw);
	
	return sw;

}

void SystemState::stop_tenants() {
	for (uint8_t i = 0; i < SystemState::tenants.size(); i++) {
		SystemState::tenants[i]->stop();

	}
}