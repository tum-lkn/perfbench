#ifndef SWITCH_EVENTS_H
#define SWITCH_EVENTS_H

#include "../OFConnection.hh"

#define SWITCH_CONN_UP 1000
#define SWITCH_CONN_DOWN 1001
#define SWITCH_GET_CONFIG_REQ 1007
#define SWITCH_FLOW_MOD 1014

using namespace fluid_base;

namespace switch_events {

class SwitchEvent {
public:
    OFConnection* ofconn;
    uint8_t* data;

    SwitchEvent(OFConnection* ofconn, int type);
    virtual ~SwitchEvent();
    int get_type();

private:
    int type;
};

class SwitchConnEstablished : public SwitchEvent {
public:
    SwitchConnEstablished(OFConnection* ofconn);
    ~SwitchConnEstablished();
};

class SwitchConnClosed : public SwitchEvent {
public:
    SwitchConnClosed(OFConnection* ofconn);
    ~SwitchConnClosed();
};

class FlowModEvent : public SwitchEvent {
public:
    OFHandler* ofhandler;
    size_t len;

    FlowModEvent(OFConnection* ofconn, OFHandler* ofhandler, void* data, size_t len);
    ~FlowModEvent();
};

class GetConfigReqEvent : public SwitchEvent {
public:
    OFHandler* ofhandler;
    size_t len;

    GetConfigReqEvent(OFConnection* ofconn, OFHandler* ofhandler, void* data, size_t len);
    ~GetConfigReqEvent();
};

}
#endif //SWITCH_EVENTS_H
