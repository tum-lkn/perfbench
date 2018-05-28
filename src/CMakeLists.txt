# BOOST
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.58 REQUIRED COMPONENTS system thread log log_setup filesystem)
include_directories(${Boost_INCLUDE_DIRS})
ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)


set(CMAKE_CXX_STANDARD 11)


set(SOURCE_FILES
        base/BaseOFConnection.cc
        base/BaseOFConnection.hh
        base/BaseOFServer.cc
        base/BaseOFServer.hh
        base/EventLoop.cc
        base/EventLoop.hh
        base/of.hh
        switch/base/BaseOFClient.cc
        switch/base/BaseOFClient.hh
        switch/datapath.cc
        switch/datapath.hh
        switch/flow.cc
        switch/flow.hh
        switch/OFClient.cc
        switch/OFClient.hh
        switch/packets.h
        switch/port.cc
        switch/port.hh
        switch/Switch.cc
        switch/Switch.hh
        switch/switch_events.h
        switch/switch_events.cpp
        testcases/echorequest.cpp
        testcases/featuresrequest.cpp
        testcases/flowmod.cpp
        testcases/flowmodsandwich.cpp
        testcases/flowmodsandwichsingle.cpp
        testcases/packetin.cpp
        testcases/packetinsandwich.cpp
        testcases/packetinsandwichsingle.cpp
        testcases/packetout.cpp
        testcases/statsrequestflow.cpp
        testcases/statsrequestport.cpp
        Controller.h
        Controller.cpp
        controller_events.h
        controller_events.cpp
        counter.cpp
        counter.h
        main.cpp
        of.hh
        OFConnection.cc
        OFConnection.hh
        OFServer.cc
        OFServer.hh
        OFServerSettings.cc
        OFServerSettings.hh
        packetdispatcher.cpp
        packetdispatcher.h
        packetfactory.cpp
        packetfactory.h
        reporter.cpp
        reporter.h
        rng.cpp
        rng.h
        scheduler.cpp
        scheduler.h
        signals.cpp
        signals.h
        socket.cpp
        socket.h
        systemstate.cpp
        systemstate.h
        tenant.cpp
        tenant.h
        testcases.cpp
        testcases.h
        TLS.cc
        TLS.hh
        utils.h
        perfbench.h
        perfbench.cpp
        argparser.cpp
        argparser.h
        )
set(EXECUTABLE_OUTPUT_PATH ../)
add_executable(perfbench ${SOURCE_FILES})

target_link_libraries(perfbench ${Boost_LIBRARIES} fluid_msg crafter nanomsg tbb pthread event_pthreads pcap event)