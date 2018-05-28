#ifndef PACKETS_H
#define PACKETS_H

#include <string>
#include <memory>
#include <arpa/inet.h>
#include "fluid/of10msg.hh"
#include "fluid/of13msg.hh"

using namespace fluid_msg;

class PacketFactory {
public:
	virtual OFMsg* features_request() = 0;
	virtual OFMsg* features_reply(uint8_t* buffer) = 0;

	virtual OFMsg* echo_request() = 0;
	virtual OFMsg* echo_reply(uint8_t* buffer) = 0;

	virtual OFMsg* packet_out(uint32_t out_port, uint32_t in_port, void* payload, uint16_t payload_size) = 0;

	virtual PacketInCommon* packet_in(uint32_t in_port, void* payload, uint16_t payload_size) = 0;
	virtual PacketInCommon* packet_in(uint8_t* buffer) = 0;

	virtual OFMsg* stats_request(uint8_t* buffer) = 0;
	virtual OFMsg* stats_request_flow(uint8_t table_id, uint8_t in_port_match) = 0;
	virtual OFMsg* stats_request_port(uint16_t port) = 0;
	virtual OFMsg* stats_reply(uint8_t* buffer) = 0;
	virtual OFMsg* stats_reply_desc(std::string mfr_desc_, std::string hw_desc_,
							std::string sw_desc_, std::string serial_num_, std::string dp_desc_) = 0;
	virtual OFMsg* stats_reply_port(uint8_t* buffer) = 0;
	virtual OFMsg* stats_reply_flow(uint8_t* buffer) = 0;

	virtual FlowModCommon* flow_mod(uint32_t in_port, uint32_t out_port, uint16_t priority, uint8_t table_id) = 0;
	virtual FlowModCommon* flow_mod(uint8_t* buffer) = 0;

	virtual SwitchConfigCommon* set_config(uint16_t flags, uint16_t miss_send_len) = 0;
	virtual SwitchConfigCommon* set_config(uint8_t* buffer) = 0;
	virtual SwitchConfigCommon* get_config_reply(uint16_t flags, uint16_t miss_send_len) = 0;
	virtual OFMsg* get_config_request(uint8_t* buffer) = 0;

	virtual OFMsg* barrier_request(uint8_t* buffer) = 0;
	virtual OFMsg* barrier_reply() = 0;

};

class PacketFactory10 : public PacketFactory {
public:
	OFMsg* features_request() final;
	OFMsg* features_reply(uint8_t* buffer) final;

	OFMsg* echo_request() final;
	OFMsg* echo_reply(uint8_t* buffer) final;

	OFMsg* packet_out(uint32_t out_port, uint32_t in_port, void* payload, uint16_t payload_size) final;

    PacketInCommon* packet_in(uint32_t in_port, void* payload, uint16_t payload_size) final;
	PacketInCommon* packet_in(uint8_t* buffer) final;

	OFMsg* stats_request(uint8_t* buffer) final;
	OFMsg* stats_reply_desc(std::string mfr_desc_, std::string hw_desc_,
												   std::string sw_desc_, std::string serial_num_, std::string dp_desc_) final;
	OFMsg* stats_request_flow(uint8_t table_id, uint8_t in_port_match) final;
	OFMsg* stats_request_port(uint16_t port) final;
	OFMsg* stats_reply(uint8_t* buffer) final;
	OFMsg* stats_reply_port(uint8_t* buffer) final;
	OFMsg* stats_reply_flow(uint8_t* buffer) final;

	FlowModCommon* flow_mod(uint32_t in_port, uint32_t out_port, uint16_t priority, uint8_t table_id) final;
	FlowModCommon* flow_mod(uint8_t* buffer) final;

	SwitchConfigCommon* set_config(uint16_t flags, uint16_t miss_send_len) final;
	SwitchConfigCommon* set_config(uint8_t* buffer) final;
	SwitchConfigCommon* get_config_reply(uint16_t flags, uint16_t miss_send_len) final;
	OFMsg* get_config_request(uint8_t* buffer) final;

	OFMsg* barrier_request(uint8_t* buffer) final;
	OFMsg* barrier_reply() final;
};

class PacketFactory13 : public PacketFactory {
public:
	OFMsg* features_request() final;
	OFMsg* features_reply(uint8_t* buffer) final;

	OFMsg* echo_request() final;
	OFMsg* echo_reply(uint8_t* buffer) final;

	OFMsg* packet_out(uint32_t out_port, uint32_t in_port, void* payload, uint16_t payload_size) final;

	PacketInCommon* packet_in(uint32_t in_port, void* payload, uint16_t payload_size) final;
	PacketInCommon* packet_in(uint8_t* buffer) final;

	OFMsg* stats_request(uint8_t* buffer) final;
	OFMsg* stats_reply_desc(std::string mfr_desc_, std::string hw_desc_,
							std::string sw_desc_, std::string serial_num_, std::string dp_desc_) final;

	OFMsg* stats_request_flow(uint8_t table_id, uint8_t in_port_match) final;
	OFMsg* stats_request_port(uint16_t port) final;
	OFMsg* stats_reply(uint8_t* buffer) final;
	OFMsg* stats_reply_port(uint8_t* buffer) final;
	OFMsg* stats_reply_flow(uint8_t* buffer) final;

	FlowModCommon* flow_mod(uint32_t in_port, uint32_t out_port, uint16_t priority, uint8_t table_id) final;
	FlowModCommon* flow_mod(uint8_t* buffer) final;

	SwitchConfigCommon* set_config(uint16_t flags, uint16_t miss_send_len) final;
	SwitchConfigCommon* set_config(uint8_t* buffer) final;
	SwitchConfigCommon* get_config_reply(uint16_t flags, uint16_t miss_send_len) final;
	OFMsg* get_config_request(uint8_t* buffer) final;

	OFMsg* barrier_request(uint8_t* buffer) final;
	OFMsg* barrier_reply() final;
};

#endif
#pragma once
