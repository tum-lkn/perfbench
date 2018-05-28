#ifndef ARGPARSER_H
#define ARGPARSER_H

#include <string>
#include <vector>
#include <tclap/CmdLine.h>

class ArgParser {
public:
    std::string port;
    std::string ip;
    int len;
    int max_packets;
    std::vector<int> pps;
    int sendIntvl;
    int cooldown;
    std::vector<std::string> testcases;
    std::string logfile;
    std::string reportfile;
    bool nolog;
    bool noreport;
    bool livedata;
    std::string livedata_address;
    std::string livedata_suffix;
    int ofVersion;
    std::string dataplaneIP;
    std::string dataplaneMAC;
    std::string dataplaneIntf;
    int tenants;
    bool tcpnodelay;
    std::string switchIP;
    int switchPort;
    std::string sw_type;
    int intraBurstTime;
    std::string bsizeschedDistr;
    std::string btimeschedDistr;
    std::vector<std::string> schedFiles;
    int num_switches;
    int hard_timeout;

    ArgParser();

    /**
     * Parse the command line given by argv
     * @param argc Number of arguments
     * @param argv Array of parameters
     */
    void parse(int argc, char *argv[]);
};

#endif //ARGPARSER_H
