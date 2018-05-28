#include "../testcases.h"

using namespace testcases;

class FlowStatsRequest : public BaseTestCase {
public:
	std::string name = "FlowStatsRequest";

	void start_testcase() {	}
	virtual void ctrl_event_callback(ControllerEvent* ev) {
		BaseTestCase::ctrl_event_callback(ev);

		if (ev->get_type() == CTRL_CONN_UP) {
			this->ctrl_conn_established(ev);

		}
		else if (ev->get_type() == CTRL_STATS_REPLY ) {
			StatsReplyEvent* _ev = static_cast<StatsReplyEvent*>(ev);
			this->process_stats_reply(_ev);

		}
		else if (ev->get_type() == CTRL_MULTIPART_REPLY) {
			MultipartReplyEvent* _ev = static_cast<MultipartReplyEvent*>(ev);
			this->process_stats_reply(_ev);

		}
	}
	void ctrl_conn_established(ControllerEvent* ev) {
		BaseTestCase::ctrl_conn_established(ev);

		// Check if we have at least 2 dataport
		for (auto &conn : this->ctrl->conns) {
			if (conn->switch_ports().size() < 2) {
				BOOST_LOG_TRIVIAL(error) << "!!\tAll Switches need at least 2 dataports, aborting...";
				SystemState::tenant_failed(this->tenant);
				return;
			}
		}

		// Install a flowmod first		
		this->flow_mod();
		

		// Register event for Stats_Replies
		this->ctrl->register_for_event(boost::bind(&BaseTestCase::ctrl_event_callback, this, _1), CTRL_STATS_REPLY);
		this->ctrl->register_for_event(boost::bind(&BaseTestCase::ctrl_event_callback, this, _1), CTRL_MULTIPART_REPLY);

		this->tenant->tenant_connected();
	}
	void create_packet(int of_version) {
		std::string sw_type = this->tenant->switch_type;

		if (sw_type == "HP"){
			this->packet(this->packetfactory->stats_request_flow(200, 1));
		}
		else {
			this->packet(this->packetfactory->stats_request_flow(0, 1));
		}

	}
	void flow_mod() {
		std::string sw_type = this->tenant->switch_type;

		printf("Installing flow mod...\n");

		for (auto &conn : this->ctrl->conns) {

			// Installing a flow mod:
			//		Output all traffic coming from port 1 to port 2
			//		And other way round (e.g. ping reply)	
			{
				if (sw_type == "HP") {
					OFMsg* msg = this->packetfactory->flow_mod(
						conn->switch_ports().at(0), conn->switch_ports().at(1),
						1233 + this->tenant->id, 200);
					uint8_t* buffer;
					buffer = msg->pack();
					conn->send(buffer, msg->length());
					fluid_msg::OFMsg::free_buffer(buffer);
				}
				else {
					OFMsg* msg = this->packetfactory->flow_mod(
						conn->switch_ports().at(0), conn->switch_ports().at(1),
						1233 + this->tenant->id, 0);
					uint8_t* buffer;
					buffer = msg->pack();
					conn->send(buffer, msg->length());
					fluid_msg::OFMsg::free_buffer(buffer);
				}
			}
			{
				if (sw_type == "HP") {
					OFMsg* msg = this->packetfactory->flow_mod(
						conn->switch_ports().at(1), conn->switch_ports().at(0),
						1233 + this->tenant->id, 200);
					uint8_t* buffer;
					buffer = msg->pack();
					conn->send(buffer, msg->length());
					fluid_msg::OFMsg::free_buffer(buffer);
				}
				else {
					OFMsg* msg = this->packetfactory->flow_mod(
						conn->switch_ports().at(1), conn->switch_ports().at(0),
						1233 + this->tenant->id, 0);
					uint8_t* buffer;
					buffer = msg->pack();
					conn->send(buffer, msg->length());
					fluid_msg::OFMsg::free_buffer(buffer);
				}
			}
		}
	}
	void print_stats_reply(fluid_msg::OFMsg* msg) {
		std::vector<fluid_msg::of10::FlowStats> fs_v;
		if (this->ofversion == 10) {
			boost::shared_ptr<fluid_msg::of10::StatsReplyFlow> msg =
					boost::static_pointer_cast<fluid_msg::of10::StatsReplyFlow>(msg);
			fs_v = msg->flow_stats();
		}
		else if (this->ofversion == 13) {
			// TO-DO: Openflow 1.3 MultiStatsRequest
			return;
		}

		for (std::vector<int>::size_type i = 0; i != fs_v.size(); i++) {
			printf("FlowStats #%d:\n", (int)i);
			printf("\tIn port %d\n", (int)fs_v[i].match().in_port());
			printf("\tPacket count %d\n", (int)fs_v[i].packet_count());
			printf("\tByte count %d\n", (int)fs_v[i].byte_count());			
		}
	}
	void process_stats_reply(ControllerEvent* ev) {
		OFMsg* msg = this->packetfactory->stats_reply_flow(ev->data);

		uint32_t xid = msg->xid();
		double time = utils::timestamp();
		this->cntr->count_response(xid, time, msg->length(), msg->type(), ev->ofconn->list_index);		
		delete msg;
	}
	void dataplane_packet_received(size_t msg_size, unsigned char* data) {}
	fluid_msg::OFMsg* pre_packet_send(fluid_msg::OFMsg* msg, uint8_t identifier1, uint8_t identifier2) {return msg;}
	void pre_packet_send_dataplane() { }
};
