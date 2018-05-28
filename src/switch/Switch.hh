#ifndef SWITCH_HH
#define SWITCH_HH 1

#include <net/ethernet.h>
#include <signal.h>
#include <unordered_map>
#include <boost/function.hpp>
#include <memory>

#include "flow.hh"
#include "port.hh"
#include "OFClient.hh"
#include "../utils.h"
#include "../packetfactory.h"
#include "../testcases.h"
#include "switch_events.h"
#include <boost/format.hpp>

using namespace fluid_base;
using namespace fluid_msg;
using namespace testcases;
using namespace switch_events;


class Switch : public OFClient {
private:
	Datapath *dp;
	pthread_t *threads;
	uint16_t miss_send_len = 0;
	int of_version = 10;
	int id;

public:
	BaseTestCase* testcase;
	std::vector<SwPort> ports;
	std::unordered_map<int, std::vector<boost::function<void(SwitchEvent*)>>> event_listeners;

	~Switch();
	Switch(const int id, std::string address, const int port, std::vector<std::string> ports, uint64_t dp_id);

	bool start(bool block);
	void stop();
	void connection_callback(OFConnection* conn, OFConnection::Event event_type);
	void message_callback(OFConnection* conn, uint8_t type, void* data, size_t len);
	void register_for_event(boost::function<void(SwitchEvent*)> fn, int event_type);
	void dispatch_event(SwitchEvent* ev) ;
	void handle_get_config_req(OFConnection* conn, void* data, size_t len);
	void handle_set_config(OFConnection* conn, void* data, size_t len);
	void handle_barrier_request(OFConnection* conn, void* data, size_t len);
	void handle_stats_request(OFConnection* conn, void* data, size_t len);
	void handle_multipart_request(OFConnection* conn, void* data, size_t len);
	void handle_role_request(OFConnection* conn, void* data, size_t len);
};

#endif
#pragma once