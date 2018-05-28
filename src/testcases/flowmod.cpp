#include "../testcases.h"

using namespace testcases;

class Switch;

class FlowMod : public BaseTestCase {
public:
	std::string name = "FlowMod";

	Switch* sw;
	void start_testcase() {
		
	}	

	virtual void ctrl_event_callback(ControllerEvent* ev) {
		BaseTestCase::ctrl_event_callback(ev);

		if (ev->get_type() == CTRL_CONN_UP) {
			this->ctrl_conn_established(ev);
		}
	}

	void switch_event_callback(SwitchEvent* ev) {
		
	}

	void ctrl_conn_established(ControllerEvent* ev) {
		// Check if we have at least 2 dataport
		for ( auto &conn : this->ctrl->conns ) {
			if(conn->switch_ports().size() < 2){
				BOOST_LOG_TRIVIAL(error) << "!!\tAll Switches need at least 2 dataports, aborting...";
				SystemState::tenant_failed(this->tenant);
				return;
			}
		}

		BaseTestCase::ctrl_conn_established(ev);

		this->tenant->tenant_connected();
	}

	void switch_conn_established() {
		
	}

	void create_packet(int of_version) {
		// We have to differentiate between switch types
		std::string sw_type = this->tenant->switch_type;

		if (sw_type == "HP"){
			// The HP specific switch flowtable ids are:
			//	100 = Hardware Table
			//  200-203 = Software Table
			this->packet(this->packetfactory->flow_mod(
				this->ctrl->conns.at(0)->switch_ports().at(0), this->ctrl->conns.at(0)->switch_ports().at(1),
				1233 + this->tenant->id, 200));
		}
		else {
			this->packet(this->packetfactory->flow_mod(
				this->ctrl->conns.at(0)->switch_ports().at(0), this->ctrl->conns.at(0)->switch_ports().at(1),
				1233 + this->tenant->id, 0));
		}

	}

	void dataplane_packet_received(size_t msg_size, unsigned char* data) {}
	fluid_msg::OFMsg* pre_packet_send(fluid_msg::OFMsg* msg, uint8_t identifier1, uint8_t identifier2) {
		if(msg->version() == 1) {
			of10::FlowMod* msg_new = static_cast<of10::FlowMod *>(msg);
			of10::Match match;
			IPAddress addr;
			addr.setIPv4(msg_new->xid());
			match.nw_src(addr);
			match.in_port(1);
			msg_new->match(match);
			return msg_new;
		}
		if(msg->version() == 4) {
			of13::FlowMod* msg_new = static_cast<of13::FlowMod *>(msg);
			of13::Match* match = new of13::Match;

			// Add in_port match
			of13::InPort* in_port_match = new of13::InPort(1);
			match->add_oxm_field(in_port_match);

			// Add metadata match
			// We use the metadata match field for our xid
			of13::Metadata* metadata = new of13::Metadata;
			metadata->value(msg_new->xid());
			match->add_oxm_field(metadata);

			msg_new->match(*match);

			return msg_new;
		}
		return msg;
	}
	void pre_packet_send_dataplane() { }
};
