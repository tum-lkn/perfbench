#include "perfbench.h"

Perfbench::Perfbench(){

    /* Logging */
    boost::log::add_common_attributes();
    boost::log::add_console_log(std::cout);
    boost::log::add_file_log
            (
                    boost::log::keywords::file_name = "/tmp/perfbench.log",
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

    // Init Argparser
    this->argparser = new ArgParser;


}

Perfbench::~Perfbench() {}

void Perfbench::start() {
    BOOST_LOG_TRIVIAL(info) << "*********************************************";
    BOOST_LOG_TRIVIAL(info) << "**\tPerfbench started!";

    // Set the hard timeout
    this->hard_timeout = this->argparser->hard_timeout;

    // Create Tenants
    for (int i = 0; i < this->argparser->tenants; i++) {
        // testcase mapping
        std::string _testcase = "FeaturesRequest";
        if(this->argparser->testcases.size() == (size_t)1)
            _testcase = this->argparser->testcases[0];
        if(this->argparser->testcases.size() >= (size_t)(i+1)){
            _testcase = this->argparser->testcases[i];
        }

        // if 0 or >1 pps are defined use default value
        // if 1 pps is defined use it for all tenants
        // else every tenant uses its own pps
        int _pps = 100;
        if(this->argparser->pps.size() == (size_t)1)
            _pps = this->argparser->pps[0];
        if(this->argparser->pps.size() >= (size_t)(i+1)){
            _pps = this->argparser->pps[i];
        }

        // Every tenant uses its own schedFile arg
        // else leave it empty
        std::string schedFile;
        if(this->argparser->schedFiles.size() >= (size_t)(i+1)){
            schedFile = this->argparser->schedFiles[i];
        }

        Tenant* tenant = new Tenant(this->argparser->ip,
                                    this->argparser->port,
                                    _testcase,
                                    _pps,
                                    this->argparser->sendIntvl,
                                    this->argparser->intraBurstTime,
                                    this->argparser->len,
                                    this->argparser->max_packets,
                                    this->argparser->cooldown,
                                    this->argparser->nolog,
                                    this->argparser->noreport,
                                    this->argparser->livedata,
                                    this->argparser->livedata_suffix,
                                    this->argparser->ofVersion,
                                    this->argparser->dataplaneIP,
                                    this->argparser->dataplaneMAC,
                                    this->argparser->dataplaneIntf,
                                    this->argparser->logfile,
                                    this->argparser->tcpnodelay,
                                    this->argparser->switchIP,
                                    this->argparser->switchPort,
                                    this->argparser->sw_type,
                                    this->argparser->bsizeschedDistr,
                                    this->argparser->btimeschedDistr,
                                    schedFile,
                                    this->argparser->num_switches);

        tenants.push_back(tenant);
        this->systemstate->register_tenant(tenant);

    }

    // Start all tenants
    for(auto const& t: SystemState::tenants)
        t->start();

    // Init Reporter
    this->reporter->init(this->argparser->noreport, this->argparser->reportfile);

    // Init Systemstate
    this->systemstate->cooldown_time = this->argparser->cooldown;
    this->systemstate->init();

    // Wait until systemstate says finished
    int time_passed = 0;
    while (time_passed < this->hard_timeout) {
        if (this->systemstate->finished) {
            break;
        }

        else if (this->systemstate->failed) {
            BOOST_LOG_TRIVIAL(info) << "**\tRUN FAILED! Aborting...";
            break;
        }
        time_passed += 1;
        usleep(1000000);
    }
}