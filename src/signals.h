#ifndef SIGNALS_H
#define SIGNALS_H 1

#include <boost/signals2.hpp>
#include <boost/bind.hpp>

using namespace boost::signals2;

class Tenant;

/**
 * Signals Class that holds all valid events
 * Using the boost::signals2 library
 */
class Signals {
public:
	/**
	 * When a tenant connected (OF handshake done)
	 */
	static boost::signals2::signal<void(Tenant* tenant)> TenantConnected;

	/**
	 * When a tenant fails (e.g. disconnect)
	 */
	static boost::signals2::signal<void(Tenant* tenant)> TenantFailed;

	/**
	 * When all tenants finished their handshake
	 */
	static boost::signals2::signal<void()> AllTenantsConnected;

	/**
	 * When all tenants finished their handshake
	 */
	static boost::signals2::signal<void()> AllTenantsFinished;
};


#endif
#pragma once