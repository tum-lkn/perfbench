#ifndef TENANT_H
#define TENANT_H 1

#include <string>
#include <arpa/inet.h>

#include "testcases.h"
#include "Controller.h"
#include "systemstate.h"
#include "signals.h"

namespace testcases {
	class BaseTestCase;
}


/**
 * The Tenant class. Holds and creates the testcase object.
 */

class Tenant {
public:
	int id; /**< The tenants id */
	testcases::BaseTestCase* testcase; /**< The testcase */
	bool finished; /**< Tenant finished? */
	bool connected; /**< Tenants OpenFlow Handshake done? */
	std::string switch_type; /**< The switch type the tenant is using */

	/**
       * Constructor class to initialize the tenant and set up the testcase
       * @param ip the ip as a string
       * @param port the port as a string
       * @param testcase_s the testcase name
       * @param pps
       * @param send_intvl
       * @param intraBurstTime
       * @param len
       * @param max_packets
       * @param cooldown_time
       * @param nolog
       * @param noreport
       * @param livedata
       * @param livedata_suffix
       * @param ofVersion
       * @param dataplane_ip
       * @param dataplane_mac
       * @param dataplane_intf
       * @param logfile
       * @param tcpnodelay
       * @param switchIP
       * @param switchPort
       * @param sw_type
       * @param bsizeschedDistr
       * @param btimeschedDistr
       * @param schedFile
       * @param num_switches
       */
	Tenant(std::string ip, std::string port, std::string testcase_s,
		int pps, int send_intvl, int intraBurstTime, int len, int max_packets, int cooldown_time, bool nolog, bool noreport,
		bool livedata, std::string livedata_suffix, int ofVersion, std::string dataplane_ip,
		std::string dataplane_mac, std::string dataplane_intf, std::string logfile, bool tcpnodelay,
		std::string switchIP, int switchPort, std::string sw_type, std::string bsizeschedDistr, std::string btimeschedDistr, std::string schedFile,
		int num_switches);
	~Tenant();

	/**
	 * Start the tenant
	 */
	void start();
	/**
	 * Stop the tenant
	 */
    void stop();
	/**
	 * Called when tenant OF Handshake is done
	 */
	void tenant_connected();
	/**
	 * Called when tenant controller connection disconnects
	 */
	void tenant_disconnected();
	/**
	 * Called when tenant failed (e.g. connection reset)
	 */
	void tenant_failed();
	/**
	 * Called when tenant finished its testcase
	 */
	void testcase_finished();
    
	

};

#endif
#pragma once
