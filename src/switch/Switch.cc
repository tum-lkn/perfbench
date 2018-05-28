#include "Switch.hh"
#include <iostream>

uint64_t str_to_uint64(char *str){
	return strtoull (str, NULL, 16);
}

Switch::~Switch() {
    for (std::vector<SwPort*>::iterator it = this->dp->ports.begin();
         it != this->dp->ports.end(); ++it) {
        delete (*it);
    }
    delete dp;
    delete[] this->threads;
}

Switch::Switch(
        const int id = 0, std::string address = "127.0.0.1",
        const int port = 6653, std::vector<std::string> ports = std::vector<std::string>(),
        uint64_t dp_id = utils::generate_dp_id() ) :
        OFClient(id, address, port, dp_id, OFServerSettings().supported_version(4).echo_interval(600)) {
    /*Create ports threads*/
    this->id = id;

    this->dp = new Datapath();
    this->dp->conn = &this->conn;
    for (uint8_t i = 0; i < ports.size(); i++) {
        SwPort *port = new SwPort(i + 1, ports[i]);
        this->dp->ports.push_back(port);
    }
    this->threads = new pthread_t[this->dp->ports.size()];
    /*Open interfaces to capture packets*/
    for (std::vector<SwPort*>::iterator it = this->dp->ports.begin();
         it != this->dp->ports.end(); ++it) {
        (*it)->open();
        struct pcap_datapath *pcap_dp = new pcap_datapath();
        pcap_dp->dp = &this->dp;
        pcap_dp->port_no = (*it)->port_no();
        pcap_dp->pcap_ = (*it)->pcap();
        pthread_create(&threads[(*it)->port_no() - 1], NULL,
                       SwPort::pcap_capture, (void*)pcap_dp);
    }
}

bool Switch::start(bool block = false) {
    return OFClient::start(block);
}

void Switch::stop() {
    for (std::vector<SwPort*>::iterator it = this->dp->ports.begin();
         it != this->dp->ports.end(); ++it) {
        (*it)->close();
        void *pcap_dp;
        pthread_join(threads[(*it)->port_no() - 1], &pcap_dp);
        delete (struct pcap_datapath*) pcap_dp;
    }
    OFClient::stop();
}

void Switch::connection_callback(OFConnection* conn, OFConnection::Event event_type) {
    if (event_type == OFConnection::EVENT_CLOSED) {
        dispatch_event(new SwitchConnClosed(conn));
        OFClient::stop_conn();
        OFClient::start_conn();
    }
    else if (event_type == OFConnection::EVENT_ESTABLISHED) {
        dispatch_event(new SwitchConnEstablished(conn));
        BOOST_LOG_TRIVIAL(info) << "**\t[DP] Switch 0x"<< std::hex << this->datapath_id << std::dec << " connected to HV/Ctrl!";
    }

}

void Switch::message_callback(OFConnection* conn, uint8_t type, void* data, size_t len) {
    // OF 1.0
    if(conn->get_version() == 1) {
        if (type == of10::OFPT_BARRIER_REQUEST) {
            //this->dp->handle_barrier_request((uint8_t*)data);
            handle_barrier_request(conn, data, len);
        }
        if (type == of10::OFPT_PACKET_OUT) {
            //this->dp->handle_packet_out((uint8_t*)data);
        }
        if (type == of10::OFPT_FLOW_MOD) {
            //this->dp->handle_flow_mod((uint8_t*)data);
            dispatch_event(new FlowModEvent(conn, this, data, len));
        }
        if (type == of10::OFPT_STATS_REQUEST)
        {
            handle_stats_request(conn, data, len);
        }
        if (type == of10::OFPT_GET_CONFIG_REQUEST) {
            handle_get_config_req(conn, data, len);
            //dispatch_event(new GetConfigReqEvent(conn, this, data, len));
        }
        if (type == of10::OFPT_SET_CONFIG) {
            handle_set_config(conn, data, len);
        }
    }
    // OF 1.3
    else if(conn->get_version() == 4) {
        if (type == of13::OFPT_BARRIER_REQUEST) {
            //this->dp->handle_barrier_request((uint8_t*)data);
            handle_barrier_request(conn, data, len);
        }
        if (type == of13::OFPT_PACKET_OUT) {
            //this->dp->handle_packet_out((uint8_t*)data);
        }
        if (type == of13::OFPT_FLOW_MOD) {
            //this->dp->handle_flow_mod((uint8_t*)data);
            dispatch_event(new FlowModEvent(conn, this, data, len));
        }
        if (type == of13::OFPT_MULTIPART_REQUEST)
        {
            handle_multipart_request(conn, data, len);
        }
        if (type == of13::OFPT_GET_CONFIG_REQUEST) {
            handle_get_config_req(conn, data, len);
            //dispatch_event(new GetConfigReqEvent(conn, this, data, len));
        }
        if (type == of13::OFPT_SET_CONFIG) {
            handle_set_config(conn, data, len);
        }
        if (type == of13::OFPT_ROLE_REQUEST) {
            handle_role_request(conn, data, len);
        }
    }


}

void Switch::register_for_event(boost::function<void(SwitchEvent*)> fn, int event_type) {
    //printf("register_for_event, event: %d\n", event_type);
    event_listeners[event_type].push_back(fn);

}

void Switch::dispatch_event(SwitchEvent* ev) {
    for (uint8_t i = 0; i < event_listeners[ev->get_type()].size();i++) {
        event_listeners[ev->get_type()][i](ev);
    }
    //printf("event_listeners.size (event-type: %d): %lu\n", ev->get_type(), event_listeners[ev->get_type()].size());
    //delete ev;
}

void Switch::handle_get_config_req(OFConnection* conn, void* data, size_t len) {
    // Create getconfigrequest from raw data
    auto get_config_req = this->testcase->packetfactory->get_config_request((uint8_t*)data);
    uint32_t xid = get_config_req->xid();

    // Send getconfigreply
    SwitchConfigCommon* msg = this->testcase->packetfactory->get_config_reply(0, this->miss_send_len);
    msg->xid(xid);
    uint8_t* buffer;
    buffer = msg->pack();
    conn->send(buffer, msg->length());
    fluid_msg::OFMsg::free_buffer(buffer);

}

void Switch::handle_set_config(OFConnection* conn, void* data, size_t len) {
    // Parse setconfig from data
    SwitchConfigCommon* set_config = this->testcase->packetfactory->set_config((uint8_t*)data);
    this->miss_send_len = set_config->miss_send_len();

}

void Switch::handle_barrier_request(OFConnection* conn, void* data, size_t len) {
    // Create barrier_request from raw data
    auto req = this->testcase->packetfactory->barrier_request((uint8_t*)data);
    uint32_t xid = req->xid();

    // Send barrier_reply
    OFMsg* msg = this->testcase->packetfactory->barrier_reply();
    msg->xid(xid);
    uint8_t* buffer;
    buffer = msg->pack();
    conn->send(buffer, msg->length());
    fluid_msg::OFMsg::free_buffer(buffer);

}

void Switch::handle_stats_request(OFConnection* conn, void* data, size_t len) {
    // OF 1.0 only!

    auto req = dynamic_cast<of10::StatsRequest*>
            (this->testcase->packetfactory->stats_request((uint8_t*)data));
    uint32_t xid = req->xid();
    uint16_t type = req->stats_type();
    // Check for Desc type
    if (type == 0) {
        // Send desc stats reply
        auto msg = this->testcase->packetfactory->stats_reply_desc("perfbench_sw", "software",
                                                                     "perfbench_sw", "1234", "perfbench_sw");
        msg->xid(xid);
        uint8_t* buffer;
        buffer = msg->pack();
        conn->send(buffer, msg->length());
        fluid_msg::OFMsg::free_buffer(buffer);
    }

}

void Switch::handle_multipart_request(OFConnection* conn, void* data, size_t len) {
    // Create stats_request from raw data
    auto* req = dynamic_cast<of13::MultipartRequest*>
    (this->testcase->packetfactory->stats_request((uint8_t*)data));
    uint32_t xid = req->xid();
    uint16_t type = req->mpart_type();
    // Check for multipart type
    if (type == 0) {
        // desc
        auto msg = this->testcase->packetfactory->stats_reply_desc("perfbench_sw", "software",
                                                                     "perfbench_sw", "1234", "perfbench_sw");
        msg->xid(xid);
        uint8_t* buffer;
        buffer = msg->pack();
        conn->send(buffer, msg->length());
        fluid_msg::OFMsg::free_buffer(buffer);

    }
    if (type == 13) {
        // portdesc
        auto msg = new of13::MultipartReplyPortDescription();
        msg->xid(xid);

        // Local Openflow port
        msg->add_port(of13::Port(of13::OFPP_LOCAL, EthAddress("000000000000"), (boost::format("pb%d") % this->id).str(),
                0x00000001, 0x00000001, 0, 0, 0, 0, 0, 0));

        // Dataplane ports
        for(auto& p : this->dp->ports) {
            msg->add_port(of13::Port((uint32_t)p->port_no(), EthAddress("000000000000"), p->name(),
                                     0x00000000, 0x00000000, 0x00002820, 0x0000282f, 0x0000282f, 0x00000000, 1000000, 1000000));
        }
        uint8_t* buffer;
        buffer = msg->pack();
        conn->send(buffer, msg->length());
        fluid_msg::OFMsg::free_buffer(buffer);
    }
    if (type == 11) {
        // meter_features
        auto msg = new of13::MultipartReplyMeterFeatures();
        msg->xid(xid);

        msg->meter_features(of13::MeterFeatures(0, 0x00000000, 0x00000000, 0, 0));

        uint8_t* buffer;
        buffer = msg->pack();
        conn->send(buffer, msg->length());
        fluid_msg::OFMsg::free_buffer(buffer);
    }

}

void Switch::handle_role_request(OFConnection* conn, void* data, size_t len) {
    auto req = new of13::RoleRequest();
    req->unpack((uint8_t*)data);

    uint32_t xid = req->xid();
    uint32_t role = req->role();

    auto msg = new of13::RoleReply(xid, role, 0x0000000000000000);

    uint8_t* buffer;
    buffer = msg->pack();
    conn->send(buffer, msg->length());
    fluid_msg::OFMsg::free_buffer(buffer);

}