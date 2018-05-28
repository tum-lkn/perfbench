#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "socket.h"
#include "testcases.h"
#include "utils.h"

using namespace boost::asio::ip;

using namespace udp_socket;

server::server(boost::asio::io_service& io_service, const std::string& ip,
	const std::string& port,
	testcases::BaseTestCase *callback_class) :
	io_service_(io_service), socket_(io_service,
		udp::endpoint(boost::asio::ip::address_v4::from_string(ip), std::stoi(port)))
{
	std::cout << "UDP Server started on " << ip << ":" << port << "\n";

	this->callback_class = callback_class;

}


server::~server() {
	std::cout << "Closing udp socket!" << "\n";
	socket_.close();
};

void server::start_receive() {
	std::cout << "Waiting for data" << "\n";

	socket_.async_receive_from(
		boost::asio::buffer(data_, max_length), endpoint_,
		boost::bind(&server::handle_receive_from, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));

}

void server::handle_receive_from(const boost::system::error_code& error,
	size_t bytes_recvd)
{
	if (!error && bytes_recvd > 0)
	{
		// Print Received Packets

		boost::asio::mutable_buffer b1 = boost::asio::buffer(data_, max_length);
		unsigned char* data = boost::asio::buffer_cast<unsigned char*>(b1);
		/*
		std::cout << "Received " << bytes_recvd << " Bytes" << "\n";
		std::cout << "Data: " << p1 << "\n";
		std::cout << "\n";
		*/

		// Call callback function
		this->callback_class->dataplane_packet_received(bytes_recvd, data);

		socket_.async_receive_from(
			boost::asio::buffer(data_, max_length), endpoint_,
			boost::bind(&server::handle_receive_from, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}
	else
	{
		std::cout << error << "\n";
	}
}

void server::send_raw_data(void* raw_data, std::size_t size) {
	this->socket_.send(boost::asio::buffer(raw_data, size));
}

void server::status() {
	std::cout << "Status:" << "\n";
}
void server::sent_data_handler(const boost::system::error_code& error, size_t bytes_sent) {}



/* UDP Client */

client::client(boost::asio::io_service& io_service, const std::string& ip,
	const std::string& port, testcases::BaseTestCase *callback_class) :
	io_service_(io_service),
	socket_(io_service, udp::endpoint(boost::asio::ip::address_v4::from_string(ip), std::stoi(port)))
{

	boost::asio::socket_base::broadcast option(true);
	socket_.set_option(option);
	std::cout << "UDP Client started" << "\n";

	this->callback_class = callback_class;
}


client::~client() {
	std::cout << "Closing udp client!" << "\n";
	socket_.close();
};

void client::send_data(const std::string& msg,
	const std::string& ip, const std::string& port) {


	try {
		this->socket_.send_to(boost::asio::buffer(msg, msg.size()),
			udp::endpoint(boost::asio::ip::address_v4::from_string(ip), std::stoi(port)));
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
}

void client::status() {
	std::cout << "Status:" << "\n";
}
void client::sent_data_handler(const boost::system::error_code& error, size_t bytes_sent) {}

/* Raw Socket */
bool raw_socket::created = false;
int raw_socket::socket_id = 0;
std::string raw_socket::iface = "";
uint32_t raw_socket::proto_id = 0;

void raw_socket::create(std::string iface, uint32_t proto_id) {
	if (!raw_socket::created) {
		raw_socket::socket_id = Crafter::SocketSender::RequestSocket(iface, proto_id);
		raw_socket::created = true;
	}
}
void raw_socket::send_data(uint8_t *pkt, size_t pkt_len) {
	if (raw_socket::created) {
		Crafter::SocketSender::SendSocket(raw_socket::socket_id, raw_socket::proto_id, pkt, pkt_len);
	}
}

