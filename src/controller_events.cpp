#include "controller_events.h"
namespace controller_events {

ControllerEvent::ControllerEvent(OFConnection* ofconn, int type) {
    this->ofconn = ofconn;
    this->type = type;
}
ControllerEvent::~ControllerEvent() {}
int ControllerEvent::get_type() {
    return this->type;
}

ControllerConnEstablished::ControllerConnEstablished(OFConnection* ofconn) :
        ControllerEvent(ofconn, CTRL_CONN_UP) {}

ControllerConnClosed::ControllerConnClosed(OFConnection* ofconn) :
        ControllerEvent(ofconn, CTRL_CONN_DOWN) {}

PacketInEvent::PacketInEvent(OFConnection *ofconn, OFHandler *ofhandler, void *data, size_t len):
        ControllerEvent(ofconn, CTRL_PACKET_IN){
    this->data = (uint8_t*)data;
    this->len = len;
    this->ofhandler = ofhandler;
}
PacketInEvent::~PacketInEvent() {
    this->ofhandler->free_data(this->data);
}

EchoRequestEvent::EchoRequestEvent(OFConnection* ofconn, OFHandler* ofhandler, void* data, size_t len) :
            ControllerEvent(ofconn, CTRL_ECHO_REQUEST) {
    this->data = (uint8_t*)data;
    this->len = len;
    this->ofhandler = ofhandler;
}
EchoRequestEvent::~EchoRequestEvent() {
    this->ofhandler->free_data(this->data);
}


EchoReplyEvent::EchoReplyEvent(OFConnection* ofconn, OFHandler* ofhandler, void* data, size_t len) :
            ControllerEvent(ofconn, CTRL_ECHO_REPLY) {
    this->data = (uint8_t*)data;
    this->len = len;
    this->ofhandler = ofhandler;
}
EchoReplyEvent::~EchoReplyEvent() {
    this->ofhandler->free_data(this->data);
}


FeaturesReplyEvent::FeaturesReplyEvent(OFConnection* ofconn, OFHandler* ofhandler, void* data, size_t len) :
            ControllerEvent(ofconn, CTRL_FEATURES_REPLY) {
    this->data = (uint8_t*)data;
    this->len = len;
    this->ofhandler = ofhandler;
}
FeaturesReplyEvent::~FeaturesReplyEvent() {
    this->ofhandler->free_data(this->data);
}


StatsReplyEvent::StatsReplyEvent(OFConnection* ofconn, OFHandler* ofhandler, void* data, size_t len) :
            ControllerEvent(ofconn, CTRL_STATS_REPLY) {
    this->data = (uint8_t*)data;
    this->len = len;
    this->ofhandler = ofhandler;
}
StatsReplyEvent::~StatsReplyEvent() {
    this->ofhandler->free_data(this->data);
}

MultipartReplyEvent::MultipartReplyEvent(OFConnection* ofconn, OFHandler* ofhandler, void* data, size_t len) :
            ControllerEvent(ofconn, CTRL_MULTIPART_REPLY) {
    this->data = (uint8_t*)data;
    this->len = len;
    this->ofhandler = ofhandler;
}
MultipartReplyEvent::~MultipartReplyEvent() {
    this->ofhandler->free_data(this->data);
}

ErrorEvent::ErrorEvent(OFConnection* ofconn, OFHandler* ofhandler, void* data, size_t len) :
            ControllerEvent(ofconn, CTRL_ERROR) {
    this->data = (uint8_t*)data;
    this->len = len;
    this->ofhandler = ofhandler;
}
ErrorEvent::~ErrorEvent() {
    this->ofhandler->free_data(this->data);
}

}