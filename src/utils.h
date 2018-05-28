#ifndef __UTILS_H__
#define __UTILS_H__

#include <sys/time.h>
#include <arpa/inet.h>
#include <cstddef>


#include "socket.h"

namespace utils {
    /**
     * Returns UNIX Timestamp in seconds
     * @return timestamp
     */
	inline double timestamp() {
		struct timeval tp;
		gettimeofday(&tp, NULL);
		double t = (double)tp.tv_sec + (double)tp.tv_usec / 1000000.0;
		return t;
	};

    /**
     * Increments IPv4 address by 1
     * @param address_string ipv4 address
     * @param increment number to increment
     * @return incremented ipv4 address
     */
	inline char* increment_address(const char* address_string, int increment)
	{
		// convert the input IP address to an integer
		in_addr_t address = inet_addr(address_string);

		// add one to the value (making sure to get the correct
		// byte orders)
		address = ntohl(address);
		address += increment;
		address = htonl(address);

		// pack the address into the struct inet_ntoa expects
		struct in_addr address_struct;
		address_struct.s_addr = address;

		// convert back to a string
		return inet_ntoa(address_struct);
	}

    /**
     * Generate random MAC address
     * @return mac
     */
	inline uint64_t generate_dp_id() {
		srand(time(NULL));
		return (((uint64_t)rand() << 0) & 0x000000000000FFFFull) ^
			(((uint64_t)rand() << 16) & 0x00000000FFFF0000ull) ^
			(((uint64_t)rand() << 32) & 0x0000FFFF00000000ull) ^
			(((uint64_t)rand() << 48) & 0xFFFF000000000000ull);
	}

    /**
     * Start a UDP socket
     * @param io_service
     * @param ip listening ip
     * @param port listening port
     * @param callback_class class to callback if packet arrives
     * @return
     */
	inline udp_socket::server* startUDPServer(
		boost::asio::io_service &io_service,
		std::string ip,	std::string port,
		testcases::BaseTestCase *callback_class)
	{
		
		udp_socket::server *s = new udp_socket::server(io_service,
			ip, port, callback_class);

		return s;
	}

    /**
     * Start UDP client
     * @param io_service
     * @param ip connect to ip
     * @param port
     * @param callback_class
     * @return
     */
	inline udp_socket::client* startUDPClient(
		boost::asio::io_service &io_service,
		std::string ip, std::string port,
		testcases::BaseTestCase *callback_class)
	{

		udp_socket::client *s = new udp_socket::client(io_service,
			ip, port, callback_class);

		return s;
	}

    /**
     * Start a raw socket (needs sudo rights)
     * @param iface interface to listen on
     * @param proto_id
     */
	inline void startRawSocket(std::string iface, uint32_t proto_id)
	{
		raw_socket::create(iface, proto_id);		
	}
}

#endif
#pragma once