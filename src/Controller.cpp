#include "Controller.h"

Controller::Controller(
        const char* address = "0.0.0.0", const int port = 6653, const int n_workers = 4, bool secure = false) :
            OFServer(address, port, n_workers, secure, OFServerSettings().supported_version(4).
                    keep_data_ownership(false).echo_interval(120).liveness_check(false).handshake(true)) {
    this->set_config(ofsc);
    this->running = true;
    size_t address_len = strlen(address) + 1;
    this->address = new char[address_len];
    memcpy(this->address, address, address_len);
    snprintf(this->port, 6, "%d", port);
}

void Controller::register_for_event( boost::function<void(ControllerEvent*)> fn, int event_type) {
    this->event_listeners[event_type].push_back(fn);
}

void Controller::set_nodelay(bool nodelay) {
    this->no_delay = nodelay;
    OFServer::set_nodelay(nodelay);
}

void Controller::stop() {
    this->running = false;
    OFServer::stop();
}


void Controller::message_callback(OFConnection* ofconn, uint8_t type, void* data, size_t len) {
    if (type == 10) { // OFPT_PACKET_IN
        dispatch_event(new PacketInEvent(ofconn, this, data, len));
    }
    else if (type == 6) { // OFPT_FEATURES_REPLY
        // If handshake already done, process feat_reply
        if (ofconn->get_state() == OFConnection::STATE_RUNNING) {

            dispatch_event(new FeaturesReplyEvent(ofconn, this, data, len));
        }
            // If handshake NOT done, process it
        else {
            // If feat reply part of the handshake
            ofconn->set_version(((uint8_t*) data)[0]);

            if(ofconn->get_version() == 1){
                fluid_msg::of10::FeaturesReply msg;
                msg.unpack((uint8_t*)data);
                std::vector<fluid_msg::of10::Port> ports = msg.ports();
                ofconn->switch_ports_10(ports);

                BOOST_LOG_TRIVIAL(info) << "**\t[CP] New Switch Connection: " << this->address << ":"
                                        << this->port << ", OF Version: "
                                        << (int)ofconn->get_version() << ", Switch Ports: " << (int)ofconn->switch_ports().size();
                ofconn->set_state(OFConnection::STATE_RUNNING);
                this->conns.push_back(ofconn);
                this->connection_callback(ofconn, OFConnection::EVENT_ESTABLISHED);


            }
            else if (ofconn->get_version() == 4){
                // Send Multipart Port_Desc
                fluid_msg::of13::MultipartRequestPortDescription msg;
                uint8_t* buffer;
                buffer = msg.pack();
                ofconn->send(buffer, msg.length());


            }
            // Add the new connection to our list


        }
    }
    else if (type == 2) { // OFPT_ECHO_REQUEST
        dispatch_event(new EchoRequestEvent(ofconn, this, data, len));
    }
    else if (type == 3) { // OFPT_ECHO_REPLY
        dispatch_event(new EchoReplyEvent(ofconn, this, data, len));
    }
    else if (type == 17) { // OFPT_STATS_REPLY
        dispatch_event(new StatsReplyEvent(ofconn, this, data, len));
    }
    else if (type == 19) { // OFPT_MULTIPART_REPLY
        if (ofconn->get_state() == OFConnection::STATE_RUNNING) {
            // If handshake already done, process feat_reply
            dispatch_event(new MultipartReplyEvent(ofconn, this, data, len));

        }
        else {
            fluid_msg::of13::MultipartReplyPortDescription msg;
            msg.unpack((uint8_t*)data);
            std::vector<fluid_msg::of13::Port> ports = msg.ports();
            ofconn->switch_ports_13(ports);

            BOOST_LOG_TRIVIAL(info) << "**\tSwitch Handshake done: " << this->address << ":"
                                    << this->port << ", OF Version: "
                                    << (int)ofconn->get_version() << ", Switch Ports: " << (int)ofconn->switch_ports().size();

            ofconn->set_state(OFConnection::STATE_RUNNING);
            this->conns.push_back(ofconn);
            this->connection_callback(ofconn, OFConnection::EVENT_ESTABLISHED);

        }

    }
    else if (type == 0) { // OFPT_HELLO

    }
    else if (type == 1) { // OFPT_ERROR
        //dispatch_event(new ErrorEvent(ofconn, this, data, len));
        fluid_msg::of10::Error *error = new fluid_msg::of10::Error();
        error->unpack((uint8_t*)data);
        //std::cout << "OF ERROR: " << "Type: " << error->err_type() << " Code: " << error->code() <<  "\n";
    }
}

void Controller::connection_callback(OFConnection* ofconn, OFConnection::Event type) {
    if (type == OFConnection::EVENT_STARTED) {
        //printf("Connection id=%d started\n", ofconn->get_id());

    }
    else if (type == OFConnection::EVENT_ESTABLISHED) {
        //printf("Connection id=%d established\n", ofconn->get_id());

        dispatch_event(new ControllerConnEstablished(ofconn));

    }

    else if (type == OFConnection::EVENT_FAILED_NEGOTIATION) {
        //printf("Connection id=%d failed version negotiation\n", ofconn->get_id());
    }
    else if (type == OFConnection::EVENT_CLOSED) {
        //printf("Connection id=%d closed by the user\n", ofconn->get_id());

        dispatch_event(new ControllerConnClosed(ofconn));
    }

    else if (type == OFConnection::EVENT_DEAD) {
        //printf("Connection id=%d closed due to inactivity\n", ofconn->get_id());

        dispatch_event(new ControllerConnClosed(ofconn));
    }
}

void Controller::dispatch_event(ControllerEvent* ev) {
    for (uint8_t i = 0; i < event_listeners[ev->get_type()].size(); i++) {
        event_listeners[ev->get_type()][i](ev);
    }
}
