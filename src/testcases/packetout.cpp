#include <crafter.h>

#include "../testcases.h"


using namespace testcases;

class PacketOut : public BaseTestCase {
public:
	std::string name = "PacketOut";

	Crafter::Packet* packet_headers;
	void start_testcase() {
		// Start our udp socket to receive packets from the switch
		boost::thread t(boost::bind(&PacketOut::start_data_socket, this));
	}
	virtual void ctrl_event_callback(ControllerEvent* ev) {
		BaseTestCase::ctrl_event_callback(ev);

		if (ev->get_type() == CTRL_CONN_UP) {
			this->ctrl_conn_established(ev);
		}
	}
	void ctrl_conn_established(ControllerEvent* ev) {
		// Check if we have at least 2 dataport
		for (auto &conn : this->ctrl->conns) {
			if (conn->switch_ports().size() < 2) {
				BOOST_LOG_TRIVIAL(error) << "!!\tAll Switches need at least 2 dataports, aborting...";
				SystemState::tenant_failed(this->tenant);
				return;
			}
		}

		BaseTestCase::ctrl_conn_established(ev);

		this->tenant->tenant_connected();
	}
	void create_packet_frame() {
		// --- Create UDP Packet as payload ---
		// Create a MAC Header
		Crafter::Ethernet ether_header;

		ether_header.SetDestinationMAC(this->dataplane_mac());
		ether_header.SetSourceMAC("00:00:00:00:00:00");

		// Create an IP header
		Crafter::IP ip_header;

		// Set the Source and Destination IP address
		// The source ip == (dst ip + 1) to make sure it's in the same subnet
		ip_header.SetSourceIP(utils::increment_address(this->dataplane_ip().c_str(), 1));
		ip_header.SetDestinationIP(this->dataplane_ip());

		// Create a TCP header
		Crafter::UDP udp_header;

		// Set the source and destination ports
		udp_header.SetSrcPort(50000 + this->tenant->id * 100);
		udp_header.SetDstPort(50000 + this->tenant->id * 100);

		// Create the packet...
		Crafter::Packet* packet = new Crafter::Packet(ether_header / ip_header / udp_header);
		this->packet_headers = packet;
		
	}
	fluid_msg::OFMsg* create_packetout(uint32_t xid) {

		// Create a payload
		Crafter::RawLayer raw_header;
		char payload[10];
		sprintf(payload, "%08d", xid);
		raw_header.SetPayload(payload);

		// Append payload to packet frame
		Crafter::Packet p = *this->packet_headers / raw_header;

		// Get raw packet data
		size_t size = p.GetSize();
		Crafter::byte* raw_data = new Crafter::byte[size];
		p.GetData(raw_data);

		// Create PacketOut with content
		fluid_msg::OFMsg* msg = this->packetfactory->packet_out(this->ctrl->conns.at(0)->switch_ports().at(0), this->ctrl->conns.at(0)->switch_ports().at(1), raw_data, size);
		msg->xid(xid);
		this->packet(msg);
		return msg;
	}
	void create_packet(int of_version) {
		this->create_packet_frame();
		this->create_packetout(0);
	}
	
	fluid_msg::OFMsg* pre_packet_send(fluid_msg::OFMsg* msg, uint8_t identifier1, uint8_t identifier2) {
		// Reassemble our packetout every time
		fluid_msg::OFMsg* msgg = this->create_packetout(msg->xid());
		return msgg;
	}

	void start_data_socket() {
		std::string port = std::to_string(50000 + this->tenant->id * 100);
		boost::asio::io_service io_service;
		this->_dataplane_server = utils::startUDPServer(io_service, this->dataplane_ip(), port, this);
		this->_dataplane_server->start_receive();
		io_service.run();	
	}

	void dataplane_packet_received(size_t msg_size, unsigned char* data) {
		double time = utils::timestamp();

		// Craft our packet from raw bytes
		Crafter::Packet packet;
		packet.PacketFromEthernet((Crafter::byte*) data, msg_size);

		// Get payload
		Crafter::RawLayer* raw = packet.GetLayer<Crafter::RawLayer>();
		std::string payload = raw->GetStringPayload();

		// Remove leading zeros
		payload.erase(0, std::min(payload.find_first_not_of('0'), payload.size() - 1));

		uint32_t xid = std::stoi(payload); //std::stoi( udp.GetStringPayload());
		
		// FIXME: Multi-Connection
		this->cntr->count_response(xid, time, (int)msg_size, 255, 0);
	}	
	void pre_packet_send_dataplane() { }
};
