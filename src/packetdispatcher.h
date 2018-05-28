#ifndef PACKETDISPATCHER_H
#define PACKETDISPATCHER_H

#include <string>
#include <memory>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp> 

#include "testcases.h"
#include "utils.h"
#include "socket.h"

using namespace testcases;

/**
 * The PacketDispatcher class is responsible for sending out
 * the packages according to the testcases scheduler and its parameters
 * Should be run in a sepparate thread
 */
class PacketDispatcher {
public:
	/**
	 * Registered testcase
	 */
	BaseTestCase* testcase;
	/**
	 * Running?
	 */
    bool send_loop_running = false;

	/**
	 * Register a testcase with this dispatcher object
	 * @param testcase
	 */
	void register_testcase(BaseTestCase* testcase);

	/**
	 * Start the loop
	 */
	void send_loop();

};

#endif
#pragma once