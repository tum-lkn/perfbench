#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <cstdlib>
#include <iostream>
#include <stdint.h>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread/future.hpp>
#include <crafter/Utils/RawSocket.h>


namespace testcases {
	class BaseTestCase;
}

using namespace boost::asio::ip;

namespace udp_socket {

	class server
	{
		testcases::BaseTestCase *callback_class;
	public:
		server(boost::asio::io_service& io_service, const std::string& ip,
			const std::string& port,
			testcases::BaseTestCase *callback_class);

		~server();
		void start_receive();
		void handle_receive_from(const boost::system::error_code& error,
			size_t bytes_recvd);

		void send_raw_data(void* raw_data, std::size_t size);
		void sent_data_handler(const boost::system::error_code& error, size_t bytes_sent);
		void status();

	private:
		boost::asio::io_service& io_service_;
		udp::socket socket_;
		udp::endpoint endpoint_;
		enum { max_length = 1024 };
		char data_[max_length];
	};

	class client
	{
		testcases::BaseTestCase *callback_class;
	public:
		client(boost::asio::io_service& io_service,
			const std::string& ip, const std::string& port,
			testcases::BaseTestCase *callback_class);

		~client();

		void send_data(const std::string& msg,
			const std::string& ip, const std::string& port);
		void sent_data_handler(const boost::system::error_code& error, size_t bytes_sent);
		void status();

	private:
		boost::asio::io_service& io_service_;
		udp::socket socket_;
		udp::endpoint endpoint_;
	};



}

/* Raw Socket*/
/* Make this raw_socket static cause we only need one instance */

class raw_socket
{
public:
	static void create(std::string iface, uint32_t proto_id);
	static void send_data(uint8_t *pkt, size_t pkt_len);

	static bool created;
	static int socket_id;
	static std::string iface;
	static uint32_t proto_id;
};
#endif
#pragma once