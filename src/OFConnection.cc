#include "base/BaseOFConnection.hh"
#include "OFConnection.hh"

namespace fluid_base {

OFConnection::OFConnection(BaseOFConnection* c, OFHandler* ofhandler) {
    this->ofhandler = ofhandler;
    this->conn = c;
    this->conn->set_manager(this);
    this->id = c->get_id();
    this->set_state(STATE_HANDSHAKE);
    this->set_alive(true);
    this->set_version(0);
    this->application_data = NULL;
}

int OFConnection::get_id() {
    return this->id;
}

bool OFConnection::is_alive() {
    return this->alive;
}

void OFConnection::set_alive(bool alive) {
    this->alive = alive;
}

uint8_t OFConnection::get_state() {
    return state;
}

void OFConnection::set_state(OFConnection::State state) {
    this->state = state;
}

uint8_t OFConnection::get_version() {
    return this->version;
}

void OFConnection::set_version(uint8_t version) {
    this->version = version;
}

OFHandler* OFConnection::get_ofhandler() {
    return this->ofhandler;
}

void OFConnection::send(void* data, size_t len) {
    if (this->conn != NULL)
        this->conn->send((uint8_t*) data, len);
}

void OFConnection::add_timed_callback(void* (*cb)(void*),
                                      int interval,
                                      void* arg) {
    if (this->conn != NULL)
        this->conn->add_timed_callback(cb, interval, arg);
}

void* OFConnection::get_application_data() {
    return this->application_data;
}

void OFConnection::set_application_data(void* data) {
    this->application_data = data;
}

void OFConnection::close() {
    // Don't close twice
    if (this->conn == NULL)
        return;

    set_state(STATE_DOWN);
    // Close the BaseOFConnection. This will trigger
    // BaseOFHandler::base_connection_callback. Then BaseOFServer will take
    // care of freeing it for us, so we can lose track of it.
    this->conn->close();
    this->conn = NULL;
}

void OFConnection::switch_ports_10(std::vector<fluid_msg::of10::Port> ports) {
    this->_switch_ports_10 = ports;
}
void OFConnection::switch_ports_13(std::vector<fluid_msg::of13::Port> ports) {
    this->_switch_ports_13 = ports;
}
std::vector<uint32_t> OFConnection::switch_ports(bool dp_only) {
    std::vector<uint32_t> ports;

    // OF 1.0
    if(this->get_version() == 1){
        for (uint8_t i = 0; i < this->_switch_ports_10.size(); i++) {
            uint32_t port_no = this->_switch_ports_10.at(i).port_no();
            // if dp_only flag = true, only return real dataports
            if (dp_only){if(port_no <= 256) ports.push_back(port_no); }
            else { ports.push_back(port_no); }
        }
    }
    // OF 1.3
    else if(this->get_version() == 4)
    {
        for (uint8_t i = 0; i < this->_switch_ports_13.size(); i++) {
            uint32_t port_no = this->_switch_ports_13.at(i).port_no();
            // if dp_only flag = true, only return real dataports
            if (dp_only){if(port_no <= 256) ports.push_back(port_no); }
            else { ports.push_back(port_no); }
        }
    }

    // Sort the ports in ascending order
    std::sort(ports.begin(), ports.end());

    return ports;
}


}
