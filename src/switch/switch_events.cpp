#include "switch_events.h"

namespace switch_events {

SwitchEvent::SwitchEvent(OFConnection* ofconn, int type) {
    this->ofconn = ofconn;
    this->type = type;
}
SwitchEvent::~SwitchEvent(){}
int SwitchEvent::get_type() {
    return this->type;
}

SwitchConnEstablished::SwitchConnEstablished(OFConnection* ofconn) : SwitchEvent(ofconn, SWITCH_CONN_UP){}
SwitchConnEstablished::~SwitchConnEstablished(){}

SwitchConnClosed::SwitchConnClosed(OFConnection* ofconn) : SwitchEvent(ofconn, SWITCH_CONN_DOWN) {}
SwitchConnClosed::~SwitchConnClosed() {}

FlowModEvent::FlowModEvent(OFConnection* ofconn, OFHandler* ofhandler, void* data, size_t len) :
            SwitchEvent(ofconn, SWITCH_FLOW_MOD) {
    this->data = (uint8_t*)data;
    this->len = len;
    this->ofhandler = ofhandler;
}
FlowModEvent::~FlowModEvent() {
    this->ofhandler->free_data(this->data);
}

GetConfigReqEvent::GetConfigReqEvent(OFConnection* ofconn, OFHandler* ofhandler, void* data, size_t len) :
        SwitchEvent(ofconn, SWITCH_GET_CONFIG_REQ) {
    this->data = (uint8_t*)data;
    this->len = len;
    this->ofhandler = ofhandler;
}
GetConfigReqEvent::~GetConfigReqEvent() {
    this->ofhandler->free_data(this->data);
}

}