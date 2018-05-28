#include "packetfactory.h"

/* Features Request / Reply */
OFMsg* PacketFactory10::features_request() {
    return new of10::FeaturesRequest;
}
OFMsg* PacketFactory13::features_request() {
    return new of13::FeaturesRequest;
}
OFMsg* PacketFactory10::features_reply(uint8_t* buffer) {
    auto msg = new of10::FeaturesReply;
    msg->unpack(buffer);
    return msg;
}
OFMsg* PacketFactory13::features_reply(uint8_t* buffer) {
    auto msg = new of13::FeaturesReply;
    msg->unpack(buffer);
    return msg;
}

/* Echo Request/Reply */
OFMsg* PacketFactory10::echo_request() {
    return new of10::EchoRequest;
}
OFMsg* PacketFactory13::echo_request() {
    return new of13::EchoRequest;
}
OFMsg* PacketFactory10::echo_reply(uint8_t* buffer) {
    auto msg = new of10::EchoReply;
    msg->unpack(buffer);
    return msg;
}
OFMsg* PacketFactory13::echo_reply(uint8_t* buffer) {
    auto msg = new of13::EchoReply;
    msg->unpack(buffer);
    return msg;
}


/* Packet Out */
OFMsg* PacketFactory10::packet_out(uint32_t out_port, uint32_t in_port, void* payload, uint16_t payload_size) {
    auto msg = new of10::PacketOut;
    msg->buffer_id(-1);
    msg->in_port(in_port);
    of10::OutputAction act = of10::OutputAction(out_port, 1024);
    msg->add_action(act);
    msg->data(payload, payload_size);
    return msg;
}
OFMsg* PacketFactory13::packet_out(uint32_t out_port, uint32_t in_port, void* payload, uint16_t payload_size){
    auto msg = new of13::PacketOut;
    msg->buffer_id(-1);
    msg->in_port(of13::OFPP_CONTROLLER);
    of13::OutputAction act = of13::OutputAction(out_port, 1024);
    msg->add_action(act);
    msg->data(payload, payload_size);
    return msg;
}


/* Packet In */
PacketInCommon* PacketFactory10::packet_in(uint32_t in_port, void* payload, uint16_t payload_size) {
    auto msg = new of10::PacketIn;
    msg->in_port(in_port);
    msg->data(payload, payload_size);
    msg->total_len(payload_size);
    msg->buffer_id(-1);
    return msg;
}
PacketInCommon* PacketFactory13::packet_in(uint32_t in_port, void* payload, uint16_t payload_size) {
    auto msg = new of13::PacketIn;
    msg->data(payload, payload_size);
    msg->total_len(payload_size);
    msg->buffer_id(-1);

    // Add in_port match
    auto in_port_match = new of13::InPort(1);
    msg->add_oxm_field(in_port_match);

    return msg;
}
PacketInCommon* PacketFactory10::packet_in(uint8_t* buffer) {
    auto msg = new of10::PacketIn;
    msg->unpack(buffer);
    return msg;
}
PacketInCommon* PacketFactory13::packet_in(uint8_t* buffer) {
    auto msg = new of13::PacketIn;
    msg->unpack(buffer);
    return msg;
}

/* StatsRequest/Reply (Flow, Port) */

// !!! Pure Stats Request/Reply only exist in OF 1.0 !!!
// Workaround for OF 1.3
OFMsg* PacketFactory10::stats_request(uint8_t* buffer) {
	auto msg = new of10::StatsRequest;
	msg->unpack(buffer);
	return msg;		
}
OFMsg* PacketFactory13::stats_request(uint8_t* buffer) {
    auto msg = new of13::MultipartRequest;
    msg->unpack(buffer);
    return msg;
}
OFMsg* PacketFactory10::stats_reply_desc(std::string mfr_desc_, std::string hw_desc_,
	std::string sw_desc_, std::string serial_num_, std::string dp_desc_) {
	auto msg = new of10::StatsReplyDesc(0, 1, mfr_desc_, hw_desc_, sw_desc_, serial_num_, dp_desc_);
	return msg;
}
OFMsg* PacketFactory13::stats_reply_desc(std::string mfr_desc_, std::string hw_desc_,
                                         std::string sw_desc_, std::string serial_num_, std::string dp_desc_) {
    auto msg = new of13::MultipartReplyDesc(0, 1, mfr_desc_, hw_desc_, sw_desc_, serial_num_, dp_desc_);
    return msg;
}

OFMsg* PacketFactory10::stats_request_flow(uint8_t table_id, uint8_t in_port_match) {
    auto msg = new of10::StatsRequestFlow;
    msg->out_port(of10::ofp_port::OFPP_NONE);
    msg->table_id(table_id);
    of10::Match m;
    m.in_port(in_port_match);
    msg->match(m);
    return msg;
}
OFMsg* PacketFactory13::stats_request_flow(uint8_t table_id, uint8_t in_port_match) {
    auto msg = new of13::MultipartRequestFlow(0, 0,
             table_id, of13::OFPP_ANY, of13::OFPG_ANY, 0, 0);
    auto port = new of13::InPort(in_port_match);
    msg->add_oxm_field(port);
    return msg;
}
OFMsg* PacketFactory10::stats_request_port(uint16_t port) {
    auto msg = new of10::StatsRequestPort;
    msg->port_no(of10::ofp_port::OFPP_NONE);
    return msg;
}
OFMsg* PacketFactory13::stats_request_port(uint16_t port) {
    auto msg = new of13::MultipartRequestPortStats;
    msg->port_no(of13::OFPP_ANY);
    msg->flags(0);
    return msg;

}
OFMsg* PacketFactory10::stats_reply_flow(uint8_t* buffer) {
    auto msg = new of10::StatsReplyFlow;
    msg->unpack(buffer);
    return msg;
}
OFMsg* PacketFactory13::stats_reply_flow(uint8_t* buffer) {
    auto msg = new of13::MultipartReplyFlow;
    msg->unpack(buffer);
    return msg;

}
OFMsg* PacketFactory10::stats_reply_port(uint8_t* buffer) {
    auto msg = new of10::StatsReplyPort;
    msg->unpack(buffer);
    return msg;
}
OFMsg* PacketFactory13::stats_reply_port(uint8_t* buffer) {
    auto msg = new of13::MultipartReplyPortStats;
    msg->unpack(buffer);
    return msg;
}
OFMsg* PacketFactory10::stats_reply(uint8_t* buffer) {
    auto msg = new of10::StatsReply;
    msg->unpack(buffer);
    return msg;
}
OFMsg* PacketFactory13::stats_reply(uint8_t* buffer) {
    auto msg = new of13::MultipartReply;
    msg->unpack(buffer);
    return msg;
}


/* Flow Mod */
FlowModCommon* PacketFactory10::flow_mod(uint32_t in_port, uint32_t out_port, uint16_t priority, uint8_t table_id) {
    auto msg = new of10::FlowMod;
    msg->command(fluid_msg::of10::OFPFC_ADD);
    msg->idle_timeout(120);
    msg->hard_timeout(240);
    msg->priority(priority);
    msg->buffer_id(-1);
    msg->out_port(of10::OFPP_NONE);

    of10::Match m;
    m.in_port(in_port);
    msg->match(m);

    of10::OutputAction act(out_port, 1024);
    msg->add_action(act);

    return msg;
}
FlowModCommon* PacketFactory13::flow_mod(uint32_t in_port, uint32_t out_port, uint16_t priority, uint8_t table_id) {
    auto msg = new of13::FlowMod();
    msg->buffer_id(-1);
    msg->priority(priority);
    msg->table_id(table_id);
    msg->out_port(of13::OFPP_ANY);
    msg->out_group(of13::OFPG_ANY);

    auto port = new of13::InPort(in_port);
    msg->add_oxm_field(port);

    of13::OutputAction act(out_port, 1024);
    of13::ApplyActions inst;
    inst.add_action(act);
    msg->add_instruction(inst);

    return msg;
}

FlowModCommon* PacketFactory10::flow_mod(uint8_t* buffer) {
    auto msg = new of10::FlowMod;
    msg->unpack(buffer);
    return msg;
}
FlowModCommon* PacketFactory13::flow_mod(uint8_t* buffer) {
    auto msg = new of13::FlowMod;
    msg->unpack(buffer);
    return msg;
}


/* Set Config */
SwitchConfigCommon* PacketFactory10::set_config(uint16_t flags, uint16_t miss_send_len) {
    auto msg = new of10::SetConfig(0, flags, miss_send_len);
    return msg;
}
SwitchConfigCommon* PacketFactory13::set_config(uint16_t flags, uint16_t miss_send_len) {
    auto msg = new of13::SetConfig(0, flags, miss_send_len);
    return msg;
}

SwitchConfigCommon* PacketFactory10::set_config(uint8_t* buffer) {
    auto msg = new of10::SetConfig;
    msg->unpack(buffer);
    return msg;
}
SwitchConfigCommon* PacketFactory13::set_config(uint8_t* buffer) {
    auto msg = new of13::SetConfig;
    msg->unpack(buffer);
    return msg;
}


/* Get Config Request*/
OFMsg* PacketFactory10::get_config_request(uint8_t* buffer) {
    auto msg = new of10::GetConfigRequest;
    msg->unpack(buffer);
    return msg;
}
OFMsg* PacketFactory13::get_config_request(uint8_t* buffer) {
    auto msg = new of13::GetConfigRequest;
    msg->unpack(buffer);
    return msg;
}


/* Get Config Reply */
SwitchConfigCommon* PacketFactory10::get_config_reply(uint16_t flags, uint16_t miss_send_len) {
    auto msg = new of10::GetConfigReply(0, flags, miss_send_len);
    return msg;
}
SwitchConfigCommon* PacketFactory13::get_config_reply(uint16_t flags, uint16_t miss_send_len) {
    auto msg = new of13::GetConfigReply(0, flags, miss_send_len);
    return msg;
}


/* Barrier Request/Reply */
OFMsg* PacketFactory10::barrier_request( uint8_t* buffer) {
    auto msg = new of10::BarrierRequest;
    msg->unpack(buffer);
    return msg;
}
OFMsg* PacketFactory13::barrier_request( uint8_t* buffer) {
    auto msg = new of13::BarrierRequest;
    msg->unpack(buffer);
    return msg;
}

OFMsg* PacketFactory10::barrier_reply() {
    auto msg = new of10::BarrierReply;
    return msg;
}
OFMsg* PacketFactory13::barrier_reply() {
    auto msg = new of13::BarrierReply;
    return msg;
}

