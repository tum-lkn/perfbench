find_package(Boost 1.58 REQUIRED COMPONENTS system thread log log_setup filesystem unit_test_framework)
include_directories(${Boost_INCLUDE_DIRS})

ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)
set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        ../src/base/BaseOFConnection.cc
        ../src/base/BaseOFConnection.hh
        ../src/base/BaseOFServer.cc
        ../src/base/BaseOFServer.hh
        ../src/base/EventLoop.cc
        ../src/base/EventLoop.hh
        ../src/base/of.hh
        ../src/switch/base/BaseOFClient.cc
        ../src/switch/base/BaseOFClient.hh
        ../src/switch/datapath.cc
        ../src/switch/datapath.hh
        ../src/switch/flow.cc
        ../src/switch/flow.hh
        ../src/switch/OFClient.cc
        ../src/switch/OFClient.hh
        ../src/switch/packets.h
        ../src/switch/port.cc
        ../src/switch/port.hh
        ../src/switch/Switch.cc
        ../src/switch/Switch.hh
        ../src/switch/switch_events.h
        ../src/switch/switch_events.cpp
        ../src/testcases/echorequest.cpp
        ../src/testcases/featuresrequest.cpp
        ../src/testcases/flowmod.cpp
        ../src/testcases/flowmodsandwich.cpp
        ../src/testcases/flowmodsandwichsingle.cpp
        ../src/testcases/packetin.cpp
        ../src/testcases/packetinsandwich.cpp
        ../src/testcases/packetinsandwichsingle.cpp
        ../src/testcases/packetout.cpp
        ../src/testcases/statsrequestflow.cpp
        ../src/testcases/statsrequestport.cpp
        ../src/Controller.h
        ../src/Controller.cpp
        ../src/controller_events.h
        ../src/controller_events.cpp
        ../src/counter.cpp
        ../src/counter.h
        ../src/of.hh
        ../src/OFConnection.cc
        ../src/OFConnection.hh
        ../src/OFServer.cc
        ../src/OFServer.hh
        ../src/OFServerSettings.cc
        ../src/OFServerSettings.hh
        ../src/packetdispatcher.cpp
        ../src/packetdispatcher.h
        ../src/packetfactory.cpp
        ../src/packetfactory.h
        ../src/reporter.cpp
        ../src/reporter.h
        ../src/rng.cpp
        ../src/rng.h
        ../src/scheduler.cpp
        ../src/scheduler.h
        ../src/signals.cpp
        ../src/signals.h
        ../src/socket.cpp
        ../src/socket.h
        ../src/systemstate.cpp
        ../src/systemstate.h
        ../src/tenant.cpp
        ../src/tenant.h
        ../src/testcases.cpp
        ../src/testcases.h
        ../src/TLS.cc
        ../src/TLS.hh
        ../src/utils.h
        ../src/perfbench.h
        ../src/perfbench.cpp
        ../src/argparser.cpp
        ../src/argparser.h
        )

add_executable(tests tests.cpp ${SOURCE_FILES})
target_link_libraries(tests ${Boost_LIBRARIES} fluid_msg crafter nanomsg tbb pthread event_pthreads pcap event)
add_test(NAME tests COMMAND tests)
