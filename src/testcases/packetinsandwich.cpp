#include "../testcases.h"

using namespace testcases;

/*

This testcase is used for multi-tenant runs with multiple switches per tenant.
It sends Packetins from the emulated switches up to the controller and calcs latency

 */

class Switch;

class PacketInSandwich : public BaseTestCase {
public:
	std::string name = "PacketInSandwich";

	Switch* sw;
	void start_testcase() {
		this->switch_sender = true;

		// Start Switch		
        this->start_switches(this->num_switches);

		// Register packetin receive event
		ctrl->register_for_event(boost::bind(&PacketInSandwich::ctrl_event_callback, this, _1), CTRL_PACKET_IN);
	}

	void start_switches(int n) {
		for ( int i = 0; i < n; i++ ) {       
			uint64_t dpid = 256 * this->tenant->id + i + 1;

			int ofsc_version = 4;
			if (this->ofversion == 10) {
				ofsc_version = 1;
			}
			OFServerSettings ofsc = OFServerSettings();
			ofsc.supported_version(ofsc_version).echo_interval(120);

			Switch* sw = SystemState::start_switch(i, this->switchIP, this->switchPort, dpid, ofsc, false);
			sw->testcase = this;
			// Register switch up event
			sw->register_for_event(boost::bind(&PacketInSandwich::switch_event_callback, this, _1), SWITCH_CONN_UP);	
			sw->register_for_event(boost::bind(&PacketInSandwich::switch_event_callback, this, _1), SWITCH_CONN_DOWN);

			sw->start(false);

			this->switches.push_back(sw);
		}

		
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

	void switch_event_callback(SwitchEvent* ev) {
		if (ev->get_type() == SWITCH_CONN_UP) {
			this->switch_conn_established(ev);
		}	
		else if(ev->get_type() == SWITCH_CONN_DOWN)	{
		}
	}

	void ctrl_conn_established(ControllerEvent* ev) {
		BaseTestCase::ctrl_conn_established(ev);
		
		// Wait for all the connections (conn-num == switch-num)
		// Tell the tenant we are connected
		if((int)this->ctrl->conns.size() == this->num_switches)
			this->tenant->tenant_connected();	
		this->send_set_config();
		
	}

	void switch_conn_established(SwitchEvent* ev) {
		if(this->switch_sender){
			int sw_id = ((OFClient*)ev->ofconn->get_ofhandler())->get_id();
			//printf("switch_conn_established: t: %d sw-id: %d\n", this->tenant->id, sw_id);

			this->cntr->conn_ids.push_back(sw_id);

			// Get the list index of the connection (list-index != conn-id)
			size_t i = this->cntr->conn_ids.size() - 1;
			ev->ofconn->list_index = i;

			//printf("new list at %d\n", (int)i);

			// Initialize Stats to 0
			this->cntr->requests.push_back(0);
			this->cntr->responses.push_back(0);
			this->cntr->last_response_xid.push_back(0);
			this->cntr->missing_responses.push_back(0);

			// Initialize Req/Resp Lists
			this->cntr->req_xids.push_back(tbb::concurrent_vector<uint32_t>());
			this->cntr->req_times.push_back(tbb::concurrent_vector<double>());
			this->cntr->req_bytes.push_back(tbb::concurrent_vector<uint32_t>());
			this->cntr->req_msgtypes.push_back(tbb::concurrent_vector<uint32_t>());

			this->cntr->resp_xids.push_back(tbb::concurrent_vector<uint32_t>());
			this->cntr->resp_times.push_back(tbb::concurrent_vector<double>());
			this->cntr->resp_bytes.push_back(tbb::concurrent_vector<uint32_t>());
			this->cntr->resp_msgtypes.push_back(tbb::concurrent_vector<uint32_t>());
		}	
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

	void create_packet(int of_version) {
		// --- Create UDP Packet as payload ---
		// Create a MAC Header
		Crafter::Ethernet ether_header;

		ether_header.SetDestinationMAC("00:00:00:00:00:00");
		ether_header.SetSourceMAC("00:00:00:00:00:00");

		// Create an IP header
		Crafter::IP ip_header;

		// Set the Source and Destination IP address		
		ip_header.SetSourceIP("10.0.0.1");
		ip_header.SetDestinationIP("10.0.0.2");

		// Create a TCP header
		Crafter::UDP udp_header;

		// Set the source and destination ports
		udp_header.SetSrcPort(60000);
		udp_header.SetDstPort(60000);

		// Create the packet...
		Crafter::Packet packet(ether_header / ip_header / udp_header);

		// Create a payload
		Crafter::RawLayer raw_header;
		char payload[9];
		sprintf(payload, "%08d", 1);
		raw_header.SetPayload(payload);

		// Append payload to packet frame
		Crafter::Packet p = packet / raw_header;

		// Get raw packet data
		size_t size = p.GetSize();
		Crafter::byte* raw_data = new Crafter::byte[size];
		p.GetData(raw_data);

		// Create PacketOut with content
		fluid_msg::OFMsg* msg = this->packetfactory->packet_in(1, raw_data, size);
		
		this->packet(msg);
	}

	void process_packet_in(PacketInEvent* ev) {
		PacketInCommon* msg = this->packetfactory->packet_in(ev->data);
		
		// Craft our packet from raw bytes
		Crafter::Packet packet;		
		packet.PacketFromEthernet((Crafter::byte*) msg->data(), msg->data_len());        

		// Get payload
		Crafter::RawLayer* raw = packet.GetLayer<Crafter::RawLayer>();
		std::string payload = raw->GetStringPayload();
		
		// Remove leading zeros
		payload.erase(0, std::min(payload.find_first_not_of('0'), payload.size() - 1));
		
		double time = utils::timestamp();
		uint32_t xid = msg->xid(); //std::stoi( udp.GetStringPayload());
		
		int conn_index = std::stoi(payload);
		
		
		this->cntr->count_response(xid, time, msg->length(), msg->type(), conn_index);
		delete msg;
	};
	void dataplane_packet_received(size_t msg_size, unsigned char* data) {}

	fluid_msg::OFMsg* pre_packet_send(fluid_msg::OFMsg* msg, uint8_t identifier1, uint8_t identifier2) {
		// Identifier1: dpid of switch
		// Identifier2: conn-list-index


		// --- Create UDP Packet as payload ---
		// Create a MAC Header
		Crafter::Ethernet ether_header;

		std::stringstream dstmac_ss;
		dstmac_ss << "00:00:00:" << std::hex << (int)this->tenant->id << ":" << std::hex << (int)identifier2+1 << ":02";
		std::string dst_mac = dstmac_ss.str();

		std::stringstream srcmac_ss;
		srcmac_ss << "00:00:00:" << std::hex << (int)this->tenant->id << ":" << std::hex << (int)identifier2+1 << ":01";
		std::string src_mac = srcmac_ss.str();

		ether_header.SetDestinationMAC(dst_mac);
		ether_header.SetSourceMAC(src_mac);

		// Create an IP header
		Crafter::IP ip_header;

		// Set the Source and Destination IP address
		// Assemble Dst/Src IP
		std::stringstream src_ip_ss;
		src_ip_ss << "10.0." << this->tenant->id << ".1";
		std::string src_ip = src_ip_ss.str();

        std::stringstream dst_ip_ss;
		dst_ip_ss << "10.0." << this->tenant->id << ".2";
		std::string dst_ip = dst_ip_ss.str();


		ip_header.SetSourceIP(src_ip);
		ip_header.SetDestinationIP(dst_ip);

		// Create a TCP header
		Crafter::UDP udp_header;

		// Set the source and destination ports
		udp_header.SetSrcPort(50000 + this->tenant->id * 100 + this->cntr->conn_ids[identifier2]);
		udp_header.SetDstPort(50000 + this->tenant->id * 100 + this->cntr->conn_ids[identifier2]);

		// Create the packet...
		Crafter::Packet packet(ether_header / ip_header / udp_header);

		// Create a payload
		Crafter::RawLayer raw_header;
		char payload[9];
		sprintf(payload, "%08d", identifier2);
		raw_header.SetPayload(payload);

		// Append payload to packet frame
		Crafter::Packet p = packet / raw_header;

		// Get raw packet data
		size_t size = p.GetSize();
		Crafter::byte* raw_data = new Crafter::byte[size];
		p.GetData(raw_data);

		// Create PacketIn with content
		fluid_msg::OFMsg* msgg = this->packetfactory->packet_in(1, raw_data, size);
		msgg->xid(msg->xid());
		
		this->packet(msgg);
		return msgg;
	}
	void pre_packet_send_dataplane() { }
};
