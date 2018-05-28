#ifndef PERFBENCH_H
#define PERFBENCH_H

#define BOOST_LOG_DYN_LINK 1

#include <vector>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#include "systemstate.h"
#include "reporter.h"
#include "argparser.h"
#include "tenant.h"

/**
 *  Perfbench main class. DESCRIPTION
 */

class Perfbench {
public:
    /**
    * The Systemstate static class
     */
    SystemState* systemstate;
    /**
    * Reporter responsible for live logging (console) and interval reporting to file
     */
    Reporter* reporter;
    /**
     * Parser for the command line
     */
    ArgParser* argparser;

    /**
     * Holds all the tenants that are created in start()
     */
    std::vector<Tenant*> tenants;

    Perfbench();
    ~Perfbench();

    /**
    * Starts Perfbench
     */
    void start();

    /**
    * Hard Timeout for the perfbench start routine
    */
    int hard_timeout;

};

#endif //PERFBENCH_H
