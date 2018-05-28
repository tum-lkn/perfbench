#include "../testcases.h"

using namespace testcases;

class EchoRequest : public BaseTestCase {
public:
	std::string name = "EchoRequest";
	
	void start_testcase() {	}
	virtual void ctrl_event_callback(ControllerEvent* ev) {
		BaseTestCase::ctrl_event_callback(ev);

		if (ev->get_type() == CTRL_CONN_UP) {
			this->ctrl_conn_established(ev);
		}
		else if (ev->get_type() == CTRL_ECHO_REPLY) {
			EchoReplyEvent* _ev = static_cast<EchoReplyEvent*>(ev);
			this->process_echo_reply(_ev);
		}
	}

	void ctrl_conn_established(ControllerEvent* ev) {
		BaseTestCase::ctrl_conn_established(ev);

		// Register event for Stats_Replies
		this->ctrl->register_for_event(boost::bind(&BaseTestCase::ctrl_event_callback, this, _1), CTRL_ECHO_REPLY);

		this->tenant->tenant_connected();
	}

	void create_packet(int of_version) {
		this->packet(this->packetfactory->echo_request());
	}

	void process_echo_reply(EchoReplyEvent* ev) {
		OFMsg* msg = this->packetfactory->echo_reply(ev->data);
		uint32_t xid = msg->xid();
		double time = utils::timestamp();
		this->cntr->count_response(xid, time, msg->length(), msg->type(), ev->ofconn->list_index);
		delete msg;
	}

	void dataplane_packet_received(size_t msg_size, unsigned char* data) {}
	fluid_msg::OFMsg* pre_packet_send(fluid_msg::OFMsg* msg, uint8_t identifier1, uint8_t identifier2) { return msg;}
	void pre_packet_send_dataplane() { }
};
