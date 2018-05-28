#define BOOST_LOG_DYN_LINK 1


#include <unistd.h>
#include <string>
#include <tclap/CmdLine.h>
#include <memory>
#include <iostream>
#include <system_error>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

#include "../../src/switch/Switch.hh"

#include <signal.h> 
volatile sig_atomic_t flag = 0;
void sig_capture(int sig){ // can be called asynchronously
    flag = 1; 
}


int main(int argc, char **argv) {
    signal(SIGINT, sig_capture); 
    
	/* Logging */
	boost::log::add_common_attributes();
	boost::log::add_console_log(std::cout);
	boost::log::add_file_log
		(
			boost::log::keywords::file_name = "perfbench.log",
			boost::log::keywords::open_mode = std::ios_base::app,
			boost::log::keywords::rotation_size = 10 * 1024 * 1024,
			//boost::log::keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(0, 0, 0),
			boost::log::keywords::format = "[%TimeStamp%]: %Message%",
			boost::log::keywords::auto_flush = true
		);

	boost::log::core::get()->set_filter
		(
			boost::log::trivial::severity >= boost::log::trivial::info
		);
	

	BOOST_LOG_TRIVIAL(info) << "*********************************************";
	BOOST_LOG_TRIVIAL(info) << "**\tPerfbench Switch started!";
    
    

	/* Cmd line parser */	
	std::string switchIP;
	int switchPort;	

	try {
	// Tool description
	TCLAP::CmdLine cmd("OpenFlow Switch", ' ', "0.1");	

	// Switch IP (switch connect to)
	TCLAP::ValueArg<std::string> switchIPArg("", "switch-ip", "Define IP where switch should connect to. Only used with FlowMod testcase", false, "127.0.0.1", "string");
	cmd.add(switchIPArg);

	// Switch Port (switch connect to)
	TCLAP::ValueArg<int> switchPortArg("", "switch-port", "Define Port where switch should connect to. Only used with FlowMod testcase", false, 6633, "int");
	cmd.add(switchPortArg);		

	// Parse the argv array.
	cmd.parse( argc, argv );
	
	switchIP = switchIPArg.getValue();
	switchPort = switchPortArg.getValue();
    
    

	} catch (TCLAP::ArgException &e)  // catch any exceptions
	{ std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; }
    
    
    /* START SWITCH */
    // Adding some port names
    uint64_t dpid = 1;
	std::vector<std::string> ports;	
    
    std::string port1 = "veth0";
    std::string port2 = "veth1";

	ports.push_back(port1);
	ports.push_back(port2);

	BOOST_LOG_TRIVIAL(info) << "**\tSwitch started! DPID: 0x"<< std::hex << dpid << std::dec <<  " Ports: " << port1 << ", "<< port2 << " Connecting to: " << switchIP << ":" << switchPort;

	Switch* sw = new Switch(0, switchIP, switchPort, ports, dpid);

	// Start switch
	sw->start(false);	
	
	

	while (1) {	
        // Exit on CTRL-C
		if(flag){
            printf("\n SIG INT caught. Stopping Switch!\n");    
            break;
        }   
	}	

	return 0;
}
