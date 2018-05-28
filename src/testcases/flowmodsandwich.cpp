#include "../testcases.h"

using namespace testcases;

/*

This testcase is used for multi-tenant runs with multiple switches per tenant.
It sends Flowmods from top to the emulated switches and calcs latency

 */


class Switch;

class FlowModSandwich : public BaseTestCase {
public:
	std::string name = "FlowModSandwich";

	Switch* sw;
	void start_testcase() {
		// Start Switch
		// We only want to start one instance of the switches
		if(this->switches.size() == (size_t)0)
			this->start_switches(this->num_switches);
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
			sw->register_for_event(boost::bind(&FlowModSandwich::switch_event_callback, this, _1), SWITCH_CONN_UP);
			// Register flow mod received event
			sw->register_for_event(boost::bind(&FlowModSandwich::switch_event_callback, this, _1), SWITCH_FLOW_MOD);

			sw->start(false);

			this->switches.push_back(sw);
		}

		
	}

	virtual void ctrl_event_callback(ControllerEvent* ev) {
		BaseTestCase::ctrl_event_callback(ev);

		if (ev->get_type() == CTRL_CONN_UP) {
			this->ctrl_conn_established(ev);
		}
	}

	void switch_event_callback(SwitchEvent* ev) {
		if (ev->get_type() == SWITCH_CONN_UP) {
			this->switch_conn_established();
		}
		else if (ev->get_type() == SWITCH_FLOW_MOD) {
			FlowModEvent* _ev = static_cast<FlowModEvent*>(ev);
			this->process_flow_mod(_ev);
		}
	}

	void ctrl_conn_established(ControllerEvent* ev) {
		BaseTestCase::ctrl_conn_established(ev);
		
		// Wait for all the connections (conn-num == switch-num)
		// Tell the tenant we are connected
		if((int)this->ctrl->conns.size() == this->num_switches)
			this->tenant->tenant_connected();
		
		
	}

	void switch_conn_established() {
		
	}

	void create_packet(int of_version) {
		FlowModCommon* msg = this->packetfactory->flow_mod(1, 2, 0, 0);

		// Set the priority field as our identifier
		msg->priority(1233 + this->tenant->id);

		this->packet(msg);
	}

	void process_flow_mod(FlowModEvent* ev) {
		double time = utils::timestamp();
		auto msg_raw = this->packetfactory->flow_mod(ev->data);
		
		// Only handle flowmods actually created by this tenant
		// We match on the priority field		
		if ((int)msg_raw->priority() != 1233 + this->tenant->id) { delete msg_raw; return; }


		// The Src IPAddr field is our req/resp identifier
		// This is some kind of workaround due to some Hypervisors rewriting XIDs
		// We have to differentiate between OF 1.0/1.3
		if(msg_raw->version() == 1) {
			of10::FlowMod* msg = static_cast<of10::FlowMod*>(msg_raw);
			of10::Match match = msg->match();
			uint32_t xid = match.nw_src().getIPv4();
			// We determine the conn-index by the xid
			int conn_index = xid / 10000000;
			this->cntr->count_response(xid, time, msg->length(), msg->type(), conn_index);
		}
		if(msg_raw->version() == 4) {
			of13::FlowMod* msg = static_cast<of13::FlowMod*>(msg_raw);
			of13::Match match = msg->match();
			of13::Metadata* metadata = match.metadata();

			uint32_t xid = (uint32_t) metadata->value();
			// We determine the conn-index by the xid
			int conn_index = xid / 10000000;
			this->cntr->count_response(xid, time, msg->length(), msg->type(), conn_index);
		}
		delete msg_raw;
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
            of13::Match match;

            // Add in_port match
            of13::InPort* in_port_match = new of13::InPort(1);
            match.add_oxm_field(in_port_match);

            // Add metadata match
            // We use the metadata match field for our xid
			of13::Metadata* metadata = new of13::Metadata;
            metadata->value(msg_new->xid());
            match.add_oxm_field(metadata);

            msg_new->match(match);

            return msg_new;
		}
		return msg;
	}
	void pre_packet_send_dataplane() { }
};
