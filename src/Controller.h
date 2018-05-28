#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include <vector>
#include <climits>
#include <unordered_map>
#include <list>
#include <boost/function.hpp>
#include <boost/log/trivial.hpp>
#include <boost/numeric/conversion/cast.hpp>

#include "OFServer.hh"
#include "OFConnection.hh"
#include "fluid/of10msg.hh"
#include "fluid/of13msg.hh"

#include "testcases.h"
#include "controller_events.h"

using namespace std;
using namespace fluid_base;
using namespace controller_events;



typedef std::unordered_map<uint64_t, uint16_t> L2TABLE;

#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

namespace testcases {
    class BaseTestCase;
}

/**
 * Controller class
 */

class Controller : public OFServer {
public:
    std::unordered_map<int, std::vector<boost::function < void(ControllerEvent*)>>> event_listeners;
    /**
     * Controller running?
     */
    bool running;

    /**
     * TCP_NO_DELAY ON/OFF
     */
    bool no_delay = false;

    /**
     * Openflow handshake with switch/hv done?
     */
    bool switch_up = false;

    /**
     * Listening address of the controller
     */
    char *address;

    /**
     * Listening port
     */
    char port[6];

    /**
     * Associated testcase
     */
    testcases::BaseTestCase* testcase;

    /**
     * Controller connections (e.g. multi-switch testcases)
     */
    std::vector<OFConnection *> conns;

    Controller(const char* address , int port, int n_workers, bool secure);

    /**
     * Register a function callback to an event type
     * @param fn function to call
     * @param event_type event type
     */
    void register_for_event(boost::function<void(ControllerEvent*)> fn, int event_type);

    /**
     * Set TCP_NO_DELAY
     * @param nodelay
     */
    void set_nodelay(bool nodelay);

    /**
     * Stop the controller
     */
    void stop();

    /**
     * Callback function for Openflow messages
     * @param ofconn OF connection object
     * @param type OF type
     * @param data pointer to data
     * @param len length of data
     */
    void message_callback(OFConnection* ofconn, uint8_t type, void* data, size_t len);

    /**
     * Connection callback
     * @param ofconn OF connection
     * @param type event type
     */
    void connection_callback(OFConnection* ofconn, OFConnection::Event type);
    void dispatch_event(ControllerEvent* ev);

};

#endif
#pragma once
