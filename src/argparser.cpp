#include "argparser.h"

ArgParser::ArgParser() {}

void ArgParser::parse(int argc, char *argv[]){
    try {
        // Tool description
        TCLAP::CmdLine cmd("Perfbench: An OpenFlow performance measurement tool for SDN switches and hypervisors", ' ', "1.0");

        // Port
        TCLAP::ValueArg<std::string> portArg("p","port","Controller Listening Port",false,"6633","int");
        cmd.add( portArg );

        // IP
        TCLAP::ValueArg<std::string> ipArg("H","ip","Listening IP",false,"0.0.0.0","string");
        cmd.add( ipArg );

        // Test length
        TCLAP::ValueArg<int> lenArg("l","length","Length of the test (seconds)",false,0,"int");
        cmd.add( lenArg );

        // Max Packets
        TCLAP::ValueArg<int> max_packetsArg("M", "max_packets", "End condition: Max Packets", false, 0, "int");
        cmd.add(max_packetsArg);

        // Packets per second
        TCLAP::MultiArg<int> ppsArg("P","pps","Packets per second per tenant",false,"int");
        cmd.add( ppsArg );

        // Send interval (in ms)
        TCLAP::ValueArg<int> sendIntvArg("", "send-interval", "Define the send interval the packets should be sent out."
                " The burst rate is then calculated by pps and send_interval", false, 1, "int");
        cmd.add(sendIntvArg);

        // Cooldown time
        TCLAP::ValueArg<int> cooldownArg("c","cooldown","Cooldown time after the test (seconds)",false,5,"int");
        cmd.add( cooldownArg );

        // Testcase
        std::vector<std::string> allowed_testcases;
        allowed_testcases.push_back("FeaturesRequest");
        allowed_testcases.push_back("EchoRequest");
        allowed_testcases.push_back("FlowStatsRequest");
        allowed_testcases.push_back("PortStatsRequest");
        allowed_testcases.push_back("PacketOut");
        allowed_testcases.push_back("PacketIn");
        allowed_testcases.push_back("FlowMod");
        allowed_testcases.push_back("FlowModSandwich");
        allowed_testcases.push_back("FlowModSandwichSingle");
        allowed_testcases.push_back("PacketInSandwich");
        allowed_testcases.push_back("PacketInSandwichSingle");
        TCLAP::ValuesConstraint<std::string> allowedTestcases(allowed_testcases);
        TCLAP::MultiArg<std::string> testcaseArg("T", "testcase", "Name of the testcase (e.g. FeaturesRequest, FlowStatsRequest, PortStatsRequest). Can be defined multiple times (for each tenant)", false, &allowedTestcases);
        cmd.add(testcaseArg);

        // Disable logging
        TCLAP::SwitchArg nologSwitch("", "disable-logging", "Disable logging to file (use this option if you are using very high pps rates)", false);
        cmd.add(nologSwitch);

        // Disable live report
        TCLAP::SwitchArg noreportSwitch("", "disable-live-report", "Disables the live reporting to console", false);
        cmd.add(noreportSwitch);

        // Logfile
        TCLAP::ValueArg<std::string> logfileArg("F", "logfile", "Specify logfile path", false, "perfbench", "string");
        cmd.add(logfileArg);

        // Interval Report File
        TCLAP::ValueArg<std::string> reportfileArg("", "interval-logfile", "Specify the path for the interval logfile. If blank interval reporting is deactivated", false, "", "string");
        cmd.add(reportfileArg);

        // Live data connection
        //TCLAP::SwitchArg livedataSwitch("", "enable-livedata", "Enables Live data transmission to specified address", false);
        //cmd.add(livedataSwitch);

        // Live data address
        //TCLAP::ValueArg<std::string> ldconnArg("", "livedata-addr", "Specify live data address", false, "127.0.0.1:9874", "string");
        //cmd.add(ldconnArg);

        // Live data suffix
        //TCLAP::ValueArg<std::string> ldSuffixArg("", "livedata-suffix", "Specify a suffix thats added to the end of the publishing topic", false, "", "string");
        //cmd.add(ldSuffixArg);

        // OpenFlow version
        std::vector<int> versions;
        versions.push_back(10);
        versions.push_back(13);
        TCLAP::ValuesConstraint<int> allowedVersions(versions);
        TCLAP::ValueArg<int> ofVersionArg("", "of-version", "Specify the Openflow version. Currently only supports '1.0' and '1.3'", false, 10, &allowedVersions);
        cmd.add(ofVersionArg);

        // Dataplane IP (e.g. PacketOut testcase)
        TCLAP::ValueArg<std::string> dataplaneIPArg("", "dataplane-ip", "Specify the dataplane IP perfbench should listen to for receiving udp packets (PacketOut testcase)", false, "10.162.149.60", "string");
        cmd.add(dataplaneIPArg);

        // Dataplane MAC (e.g. PacketOut testcase)
        TCLAP::ValueArg<std::string> dataplaneMACArg("", "dataplane-mac", "Specify the dataplane MAC address", false, "00:00:00:00:00:00", "string");
        cmd.add(dataplaneMACArg);

        // Dataplane MAC (e.g. PacketOut testcase)
        TCLAP::ValueArg<std::string> dataplaneIntfArg("", "dataplane-intf", "Specify the dataplane Interface (e.g. eth2)", false, "eth2", "string");
        cmd.add(dataplaneIntfArg);

        // Tenant count
        TCLAP::ValueArg<int> tenantsArg("", "tenants", "Specify number of tenants (Controller)", false, 1, "int");
        cmd.add(tenantsArg);

        // TCP No Delay
        TCLAP::SwitchArg tcpnodelaySwitch("", "tcp-no-delay", "Switch TCP NO_DELAY on", false);
        cmd.add(tcpnodelaySwitch);

        // Switch IP (switch connect to)
        TCLAP::ValueArg<std::string> switchIPArg("", "switch-ip", "Define IP where switch should connect to. Only used with FlowMod testcase", false, "127.0.0.1", "string");
        cmd.add(switchIPArg);

        // Switch Port (switch connect to)
        TCLAP::ValueArg<int> switchPortArg("", "switch-port", "Define Port where switch should connect to. Only used with FlowMod testcase", false, 6633, "int");
        cmd.add(switchPortArg);

        // Switch IP (switch connect to)
        TCLAP::ValueArg<std::string> switchtypeArg("", "switch-type", "Define the switch type [default|HP]", false, "default", "string");
        cmd.add(switchtypeArg);

        // Intra burst time (in microseconds)
        TCLAP::ValueArg<int> intrabursttimeArg("", "intra-burst-time", "Define the waiting time between each packet of a burst (in microseconds)", false, 0, "int");
        cmd.add(intrabursttimeArg);

        // Burst Size Distribution
        std::vector<std::string> distributions;
        distributions.push_back("uniform");
        distributions.push_back("normal");
        distributions.push_back("exponential");
        TCLAP::ValuesConstraint<std::string> allowedDistr(distributions);
        TCLAP::ValueArg<std::string> bsscheddistrArg("", "bsize-distr", "Define the scheduling distribution for the burst size (default is uniform)", false, "uniform", &allowedDistr);
        cmd.add(bsscheddistrArg);

        // Inter Burst Time Distribution
        TCLAP::ValueArg<std::string> btscheddistrArg("", "btime-distr", "Define the scheduling distribution for the inter burst time (default is uniform)", false, "uniform", &allowedDistr);
        cmd.add(btscheddistrArg);

        // Scheduling Information Files
        TCLAP::MultiArg<std::string> schedfilesArg("", "scheduling-file", "Path to scheduling file", false, "string");
        cmd.add(schedfilesArg);

        // Number of switches to start (FlowModSandwich)
        std::vector<int> allowed_num_switches;
        for(int i=1; i<=100; ++i){
            allowed_num_switches.push_back(i);
        }
        TCLAP::ValuesConstraint<int> allowed_num_switches_constr(allowed_num_switches);
        TCLAP::ValueArg<int> num_switchesArg("", "num-switches", "Define the number of switches to start. Max is 100", false, 1, &allowed_num_switches_constr);
        cmd.add(num_switchesArg);

        // Hard timeout (seconds)
        TCLAP::ValueArg<int> hard_timeoutArg("", "hard-timeout", "Define the hard timeout for perfbench (seconds). If this timeout exceeds perfbench will hard shutdown", false, 9999999, "int");
        cmd.add(hard_timeoutArg);

        // Parse the argv array.
        cmd.parse( argc, argv );

        // Get the value parsed by each arg
        this->port = portArg.getValue();
        this->ip = ipArg.getValue();
        this->len = lenArg.getValue();
        this->max_packets = max_packetsArg.getValue();
        this->pps = ppsArg.getValue();
        this->sendIntvl = sendIntvArg.getValue();
        this->cooldown = cooldownArg.getValue();
        this->testcases = testcaseArg.getValue();
        this->nolog = nologSwitch.getValue();
        this->noreport = noreportSwitch.getValue();
        this->logfile = logfileArg.getValue();
        this->reportfile = reportfileArg.getValue();
        //this->livedata = livedataSwitch.getValue();
        //this->livedata_address = ldconnArg.getValue();
        //this->livedata_suffix = ldSuffixArg.getValue();
        this->ofVersion = ofVersionArg.getValue();
        this->dataplaneIP = dataplaneIPArg.getValue();
        this->dataplaneMAC = dataplaneMACArg.getValue();
        this->dataplaneIntf = dataplaneIntfArg.getValue();
        this->tenants = tenantsArg.getValue();
        this->tcpnodelay = tcpnodelaySwitch.getValue();
        this->switchIP = switchIPArg.getValue();
        this->switchPort = switchPortArg.getValue();
        this->sw_type = switchtypeArg.getValue();
        this->intraBurstTime = intrabursttimeArg.getValue();
        this->bsizeschedDistr = bsscheddistrArg.getValue();
        this->btimeschedDistr = btscheddistrArg.getValue();
        this->schedFiles = schedfilesArg.getValue();
        this->num_switches = num_switchesArg.getValue();
        this->hard_timeout = hard_timeoutArg.getValue();



    } catch (TCLAP::ArgException &e)  // catch any exceptions
    { std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; }

}