#ifndef SYSTEMSTATE_H
#define SYSTEMSTATE_H 1

#include <string>
#include <memory>
#include <mutex>

#include <arpa/inet.h>
#include <tbb/concurrent_vector.h>
#include <boost/log/trivial.hpp>

#include "tenant.h"
#include "counter.h"
#include "signals.h"
#include "packetdispatcher.h"
#include "switch/Switch.hh"

class Tenant;
class Signals;
class Switch;

/**
 * System State Class
 */
class SystemState {
public:

	static bool finished; /**< System finished (e.g. all tenants finished) */
	static bool failed; /**< System failed (e.g. a tenant failed) */
	static bool running; /**< System is running */
	static bool switch_started; /**< Switch started? */
    static int cooldown_time; /**< Cooldown time at the end of the run */
	static std::vector<Switch*> switches; /**< Registered switches */
	static std::vector<Tenant*> tenants; /**< Tenants */

	static std::mutex sys_mutex;

	static void init();
    static int next_tenant_id(); /**< Return the next free tenant id */
	static void register_tenant(Tenant* tenant); /**< Register a Tenant to the sysstate */
	static void all_tenants_finished(); /**< Called when all tenants finished */
	static void tenant_finished(Tenant* tenant); /**< Called when a tenant finished */
	static void tenant_failed(Tenant* tenant); /**< Called when a tenant failed */
	static void tenant_connected(Tenant* tenant); /**< Called when a tenant successfully made the OF handshake */
	static void wait_for_tenants_to_connect(); /**< Loop to wait for all tenants to connect */
	static void all_tenants_connected(); /**< All Tenants connected */
    static void cooldown(); /**< Simple cooldown waiting loop */
    static void stop_tenants(); /**< Stop all tenants */
	static Switch* start_switch(int id, std::string ip, int port, uint64_t dpid, OFServerSettings ofsc, bool connect=true); /**< Start a switch */

};

#endif
#pragma once
