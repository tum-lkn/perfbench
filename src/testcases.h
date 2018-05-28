#ifndef TESTCASES_H
#define TESTCASES_H

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
#include <crafter.h>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/log/trivial.hpp>
#include <boost/filesystem.hpp>
#include <nanomsg/nn.h>
#include <nanomsg/pipeline.h>
#include <tbb/concurrent_vector.h>
#include <tbb/concurrent_queue.h>

#include "fluid/of10msg.hh"
#include "fluid/of13msg.hh"

#include "Controller.h"
#include "controller_events.h"
#include "counter.h"
#include "tenant.h"
#include "packetfactory.h"
#include "packetdispatcher.h"
#include "socket.h"
#include "utils.h"
#include "scheduler.h"
#include "switch/switch_events.h"



class Tenant;
class Counter;
class Controller;
class Switch;
class PacketDispatcher;

using namespace switch_events;
using namespace controller_events;

namespace testcases {

	class BaseTestCase {
	public:
		/**
		 * The associated controller instance
		 */
		Controller* ctrl;

		/**
		 * The counter
		 */
		Counter* cntr;

		/**
		 * Reference to the tenant using this testcase
		 */
		Tenant* tenant;

		/**
		 * The scheduler, responsible for packet-dispatching-scheduling
		 */
		Scheduler* scheduler;

		/**
		 * The packet dispatcher/sender
		 */
		PacketDispatcher* packetdispatcher;

		/**
		 * The OF packet factory
		 */
		PacketFactory* packetfactory;

		/**
		 * The associated switches for this testcase
		 */
		std::vector<Switch*> switches;

		/**
		 * The sending thread
		 */
		boost::thread* send_thread;
		/**
		 * The reporting thread (console, file)
		 */
		boost::thread* report_thread;

		/**
		 * Name of the testcase
		 */
		std::string name = "";

        /**
         * Listening IP address of the controller
         */
		std::string ip;

        /**
         * Listening port of the controller
         */
		std::string port;

        /**
         * Path to logfile
         */
		std::string logfile;

        /**
         * OpenFlow version
         */
		int ofversion;

        /**
         * Number of switches (e.g. PacketInSandwich testcase)
         */
		int num_switches;
		int _length, _max_packets;
        int _pps = 0;
        double _send_interval = 1.0;
        int _intra_burst_time = 0;
		int _burst_rate = 0;
		std::string _bsize_scheduling_distr;
		std::string _btime_scheduling_distr;
		std::string _scheduling_file;
        int cooldown_time = 0;
		double _burst_rate_real = 0.0;
		float _missing_packets_per_sendinterval = 0.0;
		float _missing_packets = 0.0;
		bool _logging, reporting;
		int _lds, _ofversion;
		std::string _ldsuf;
		std::string _dataplane_ip;
		std::string _dataplane_mac;
		std::string _dataplane_intf;
		double last_send = 0.0;
        double next_burst_time = 0.0;
        bool requesting_finished = false;
		bool finished = false;
		bool dataplane = false;
		bool switch_sender = false;
		bool tcpnodelay = false;
		std::string switchIP;
		int switchPort;
		fluid_msg::OFMsg* _packet;
		uint8_t* _dataplane_packet;
		size_t _dataplane_packet_size;
		udp_socket::server* _dataplane_server;
		raw_socket* _dataplane_client;

		/**
		 * Start the testcase
		 */
		void start();

		/**
		 * Controller event callback func
		 * @param ev
		 */
		virtual void ctrl_event_callback(ControllerEvent* ev);

		/**
		 * Controller connection/OF handshake done
		 * @param ev
		 */
		void ctrl_conn_established(ControllerEvent* ev);

		/**
		 * Start the controller
		 * @param ip listening ip
		 * @param port listening port
		 * @param ofVersion e.g. 10, 13
		 */
		void start_controller(std::string ip, std::string port, int ofVersion);

		/**
		 * Start live-data socket on address
		 * @param address
		 */
		void start_livedatasocket(std::string address);

		/**
		 * Finish requesting/sending of msgs
		 */
        void finish_requesting();

		/**
		 * Finish the testcase
		 */
		void finish();

		/**
		 * Write the logfile with the gathered data
		 * @param logfile_path path to logfile
		 */
		void write_logfile(std::string logfile_path);

		/**
		 * Handle incomming OF_ERRORS
		 * @param ev
		 */
		void process_error(ErrorEvent* ev);

		/**
		 * Print out the testcases parameters
		 */
		void log_start();

		/**
		 * Start the testcase (calling subclass)
		 */
		virtual void start_testcase() = 0;

		/**
		 * Callback if dataplane packet was received
		 * @param msg_size
		 * @param data
		 */
		virtual void dataplane_packet_received(size_t msg_size, unsigned char* data) = 0;

		/**
		 * Create the packet that is being sent by the testcase
		 * @param of_version
		 */
		virtual void create_packet(int of_version) = 0;

		/**
		 * Called just before the packet is sent
		 * @param msg current msg
		 * @param identifier1
		 * @param identifier2
		 * @return
		 */
		virtual fluid_msg::OFMsg* pre_packet_send(fluid_msg::OFMsg* msg, uint8_t identifier1, uint8_t identifier2) = 0;

		/**
		 * Called just before the dataplane packet is sent
		 */
		virtual void pre_packet_send_dataplane() = 0;

		/**
		 * Get test length
		 * @return
		 */
		int length();
		/**
		 * Set test length
		 * @param length
		 * @return
		 */
		BaseTestCase& length(const int length);
		/**
		 * Get max packets to be sent by this testcase
		 * @return
		 */
		int max_packets();
		/**
		 * Set max packets to be sent by this testcase
		 * @param max_packets
		 * @return
		 */
		BaseTestCase& max_packets(const int max_packets);
		/**
		 * Get the packets per second
		 * @return
		 */
		int pps();
		/**
		 * Set the packets per second
		 * @param intvl
		 * @return
		 */
		BaseTestCase& pps(const int intvl);
		/**
		 * Get the send interval of the packet bursts
		 * @return
		 */
		double send_interval();
		/**
		 * Set the send interval of the packet bursts
		 * @param intvl
		 * @return
		 */
		BaseTestCase& send_interval(const double intvl);
		/**
		 * Get the intra burst time (time between messages within a burst)
		 * @return
		 */
		int intra_burst_time();
		/**
		 * Set the intra burst time (time between messages within a burst)
		 * @param ibt
		 * @return
		 */
		BaseTestCase& intra_burst_time(const int ibt);
		/**
		 * Get the burst-size scheduling distribution
		 * @return
		 */
		std::string bsize_scheduling_distr();
		/**
		 * Set the burst-size scheduling distribution
		 * @param scheduling_distr
		 * @return
		 */
		BaseTestCase& bsize_scheduling_distr(const std::string scheduling_distr);
		/**
		 * Get the burst send-interval scheduling distribution
		 * @return
		 */
		std::string btime_scheduling_distr();
		/**
		 * Set the burst send-interval scheduling distribution
		 * @param scheduling_distr
		 * @return
		 */
		BaseTestCase& btime_scheduling_distr(const std::string scheduling_distr);
		/**
		 * Get the scheduling pattern file of the testcase
		 * @return
		 */
		std::string scheduling_file();
		/**
		 * Set the scheduling pattern file
		 * @param f
		 * @return
		 */
		BaseTestCase& scheduling_file(const std::string f);
		/**
		 * Set the loss threshold (currently not used)
		 * @return
		 */
		int loss_threshold();
		/**
		 * Get the loss threshold (currently not used)
		 * @param thr
		 * @return
		 */
		BaseTestCase& loss_threshold(const int thr);
		/**
		 * Get: Write packet data to file after the test?
		 * @return
		 */
		bool logging();
		/**
		 * Set: Write packet data to file after the test?
		 * @param logging
		 * @return
		 */
		BaseTestCase& logging(const bool logging);
		int livedatasocket();
		BaseTestCase& livedatasocket(int socket);
		std::string livedatasuffix();
		BaseTestCase& livedatasuffix(std::string suffix);
		std::string dataplane_ip();
		BaseTestCase& dataplane_ip(std::string ip);
		std::string dataplane_mac();
		BaseTestCase& dataplane_mac(std::string mac);
		std::string dataplane_intf();
		BaseTestCase& dataplane_intf(std::string intf);
		fluid_msg::OFMsg* packet();
		BaseTestCase& packet(fluid_msg::OFMsg* packet);
	};

	class TestCaseFactory {
	public:
		/**
		 * Create a new testcase object defined by <std::string> testcase
		 * @param testcase
		 * @return
		 */
		static BaseTestCase* get_testcase(std::string testcase);

	};
}
#endif
#pragma once
