#include <crafter.h>

#include "../testcases.h"


using namespace testcases;

class PacketIn : public BaseTestCase {
public:
	std::string name = "PacketIn";

	Crafter::Packet* packet_headers;

	void start_testcase() {		
		this->dataplane = true;		

		// Start our udp socket
		this->start_data_socket();
	}

	virtual void ctrl_event_callback(ControllerEvent* ev) {
		BaseTestCase::ctrl_event_callback(ev);

		if (ev->get_type() == CTRL_CONN_UP) {
			this->ctrl_conn_established(ev);
		}
		else if (ev->get_type() == CTRL_PACKET_IN) {
			PacketInEvent* _ev = static_cast<PacketInEvent*>(ev);
			this->process_packet_in(_ev);
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

		this->ctrl->register_for_event(boost::bind(&BaseTestCase::ctrl_event_callback, this, _1), CTRL_PACKET_IN);
		this->send_set_config();
		this->send_forward_flowmod();

		this->tenant->tenant_connected();
	}
	void send_set_config() {
		SwitchConfigCommon* msg = this->packetfactory->set_config(0, 128);
		uint8_t* buffer;
		buffer = msg->pack();
		for (auto &conn : this->ctrl->conns) {
			conn->send(buffer, msg->length());
		}
		fluid_msg::OFMsg::free_buffer(buffer);
	}
	void send_forward_flowmod() {
		std::string sw_type = this->tenant->switch_type;

		if (sw_type == "HP"){
			uint8_t table_id = 100;

			for (auto &conn : this->ctrl->conns) {
				for (uint8_t i = 0; i < conn->switch_ports().size(); i++) {
					OFMsg* msg = this->packetfactory->flow_mod(conn->switch_ports().at(i),
						of13::OFPP_CONTROLLER, 1, table_id);
					uint8_t* buffer;
					buffer = msg->pack();
					conn->send(buffer, msg->length());
					fluid_msg::OFMsg::free_buffer(buffer);
				}
			}

			
		}
		else {
			uint8_t table_id = 0;

			for (auto &conn : this->ctrl->conns) {
				for (uint8_t i = 0; i < conn->switch_ports().size(); i++) {
					OFMsg* msg = this->packetfactory->flow_mod(conn->switch_ports().at(i),
						of13::OFPP_CONTROLLER, 1, table_id);
					uint8_t* buffer;
					buffer = msg->pack();
					conn->send(buffer, msg->length());
					fluid_msg::OFMsg::free_buffer(buffer);
				}
			}
		}
	}
	void create_packet_frame() {
		// Assemble Src MAC (00:00:00:00:00:00 + tenant-id)
		std::stringstream mac_ss;
		mac_ss << "00:00:00:00:00:" << std::hex << this->tenant->id;
		std::string mac = mac_ss.str();

        // Assemble Dst/Src IP
		std::stringstream src_ip_ss;
		src_ip_ss << "10.0." << this->tenant->id << ".1";
		std::string src_ip = src_ip_ss.str();

        std::stringstream dst_ip_ss;
		dst_ip_ss << "10.0." << this->tenant->id << ".2";
		std::string dst_ip = dst_ip_ss.str();
		
		// Create a MAC Header
		Crafter::Ethernet ether_header;

		ether_header.SetDestinationMAC("00:00:00:00:00:ff");
		ether_header.SetSourceMAC(mac);

		// Create an IP header
		Crafter::IP ip_header;		
		ip_header.SetSourceIP(src_ip);
		ip_header.SetDestinationIP(dst_ip);

		// Create a TCP header
		Crafter::UDP udp_header;

		// Set the source and destination ports
		udp_header.SetSrcPort(50000 + this->tenant->id * 100);
		udp_header.SetDstPort(50000 + this->tenant->id * 100);

		// Create the packet...
		Crafter::Packet* packet = new Crafter::Packet(ether_header / ip_header / udp_header);
		this->packet_headers = packet;

	}
	void set_packet_payload() {
		// Create a payload
		Crafter::RawLayer raw_header;
		char payload[10];
		sprintf(payload, "%08d", std::accumulate(this->cntr->requests.begin(), this->cntr->requests.end(), 0));
		raw_header.SetPayload(payload);

		// Append payload to packet frame
		Crafter::Packet p = *this->packet_headers / raw_header;

		// Get raw packet data
		size_t size = p.GetSize();
		Crafter::byte* raw_data = new Crafter::byte[size];
		p.GetData(raw_data);

		// Set UDP packet and packet size
		this->_dataplane_packet = raw_data;
		this->_dataplane_packet_size = size;

	}
	void create_packet(int of_version) {
		this->create_packet_frame();
		this->set_packet_payload();
	}

	void pre_packet_send_dataplane() {
		// Reassemble our UDP packet every time
		this->set_packet_payload();	
	}

	void start_data_socket() {
		utils::startRawSocket(this->dataplane_intf(), 0x11);
		/*
		std::string port = std::to_string(1233+this->tenant->id);
		boost::asio::io_service io_service;
		this->_dataplane_client = utils::startUDPClient(io_service, this->dataplane_ip(), port, this);
		//io_service.run();
		*/
	}	

	void process_packet_in(PacketInEvent* ev) {
		PacketInCommon* msg = this->packetfactory->packet_in(ev->data);
		
		// Craft our packet from raw bytes
		Crafter::Packet packet;		
		packet.PacketFromEthernet((Crafter::byte*) msg->data(), msg->data_len());		

        // We match on the PktIn payload length
        size_t total_len = msg->total_len();

        //std::cout << "packetin: datalen " << (int)total_len;

        if(total_len != 60) return;        

		Crafter::UDP* udp_layer = packet.GetLayer<Crafter::UDP>();
        uint32_t dst_port;
        try{
		    dst_port = udp_layer->GetDstPort();
        }
        catch(...){ return; }

        //std::cout << " dstport " << dst_port << "\n";

        if(dst_port != (50000 + (uint32_t)this->tenant->id * 100)) return;

		// Get payload
		Crafter::RawLayer* raw = packet.GetLayer<Crafter::RawLayer>();
		std::string payload = raw->GetStringPayload();
		
		// Remove leading zeros
		payload.erase(0, std::min(payload.find_first_not_of('0'), payload.size() - 1));
		
		uint32_t xid = std::stoi(payload); //std::stoi( udp.GetStringPayload());
		double time = utils::timestamp();
		this->cntr->count_response(xid, time, msg->length(), msg->type(), ev->ofconn->list_index);
		delete msg;
	};
	void dataplane_packet_received(size_t msg_size, unsigned char* data) {}	
	fluid_msg::OFMsg* pre_packet_send(fluid_msg::OFMsg* msg, uint8_t identifier1, uint8_t identifier2) { return msg;}
	
};
