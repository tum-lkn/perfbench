#ifndef CONTROLLER_EVENTS_H
#define CONTROLLER_EVENTS_H

#include "OFConnection.hh"

#define CTRL_PACKET_IN 0
#define CTRL_CONN_DOWN 1
#define CTRL_CONN_UP 2
#define CTRL_ECHO_REQUEST 3
#define CTRL_ECHO_REPLY 4
#define CTRL_FEATURES_REPLY 6
#define CTRL_STATS_REPLY 8
#define CTRL_ERROR 9
#define CTRL_MULTIPART_REPLY 19

using namespace fluid_base;

namespace controller_events {

class ControllerEvent {
public:
    OFConnection* ofconn;
    uint8_t* data = 0;

    ControllerEvent(OFConnection* ofconn, int type) ;
    virtual ~ControllerEvent();
    virtual int get_type();

private:
    int type;
};

class ControllerConnEstablished : public ControllerEvent {
public:
    ControllerConnEstablished(OFConnection* ofconn);
};

class ControllerConnClosed : public ControllerEvent {
public:
    ControllerConnClosed(OFConnection* ofconn);
};

class PacketInEvent : public ControllerEvent {
public:
    OFHandler* ofhandler;
    size_t len;

    PacketInEvent(OFConnection* ofconn, OFHandler* ofhandler, void* data, size_t len);
    ~PacketInEvent();
};



class EchoRequestEvent : public ControllerEvent {
public:
    OFHandler* ofhandler;
    size_t len;

    EchoRequestEvent(OFConnection* ofconn, OFHandler* ofhandler, void* data, size_t len);
    ~EchoRequestEvent();
};

class EchoReplyEvent : public ControllerEvent {
public:
    OFHandler* ofhandler;
    size_t len;

    EchoReplyEvent(OFConnection* ofconn, OFHandler* ofhandler, void* data, size_t len);
    ~EchoReplyEvent();
};

class FeaturesReplyEvent : public ControllerEvent {
public:
    OFHandler* ofhandler;
    size_t len;

    FeaturesReplyEvent(OFConnection* ofconn, OFHandler* ofhandler, void* data, size_t len);
    ~FeaturesReplyEvent();
};

class StatsReplyEvent : public ControllerEvent {
public:
    OFHandler* ofhandler;
    size_t len;

    StatsReplyEvent(OFConnection* ofconn, OFHandler* ofhandler, void* data, size_t len);
    ~StatsReplyEvent();
};

class MultipartReplyEvent : public ControllerEvent {
public:
    OFHandler* ofhandler;
    size_t len;

    MultipartReplyEvent(OFConnection* ofconn, OFHandler* ofhandler, void* data, size_t len);
    ~MultipartReplyEvent();
};

class ErrorEvent : public ControllerEvent {
public:
    OFHandler* ofhandler;
    size_t len;

    ErrorEvent(OFConnection* ofconn, OFHandler* ofhandler, void* data, size_t len);
    ~ErrorEvent();
};

}

#endif //CONTROLLER_EVENTS_H
