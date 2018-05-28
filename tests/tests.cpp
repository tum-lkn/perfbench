#ifndef PERFBENCH_TESTS
#define PERFBENCH_TESTS

#define BOOST_TEST_MODULE Tests
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include "../src/perfbench.h"

BOOST_AUTO_TEST_SUITE(tests)

// Parser
BOOST_AUTO_TEST_CASE (Parser_Test)
{
    Perfbench *pb = new Perfbench;
	char *argv[] = {(char*)"./perfbench",
		(char*)"--testcase", (char*)"FeaturesRequest",
		(char*)"--testcase", (char*)"FlowModSandwich",
		(char*)"--tenants", (char*)"4",
		(char*)"--length", (char*)"60",
		NULL};
    int argc = 9;    
    pb->argparser->parse(argc, argv);
    BOOST_CHECK_EQUAL (pb->argparser->testcases.at(0), "FeaturesRequest");
    BOOST_CHECK_EQUAL (pb->argparser->testcases.at(1), "FlowModSandwich");
    BOOST_CHECK_EQUAL (pb->argparser->tenants, 4);
    BOOST_CHECK_EQUAL (pb->argparser->len, 60);
    delete pb;
};

// PPS and Send-Interval (if not set manually)
BOOST_AUTO_TEST_CASE (Testcase_Test)
{
    Perfbench *pb = new Perfbench;
    char *argv[] = {(char*)"./perfbench",
                    (char*)"--testcase", (char*)"EchoRequest",
                    (char*)"--tenants", (char*)"1",
                    (char*)"--pps", (char*)"500",
                    (char*)"--hard-timeout", (char*)"1",
                    NULL};
    int argc = 9;
    pb->argparser->parse(argc, argv);
    pb->start();

    BOOST_CHECK_EQUAL (pb->tenants.at(0)->testcase->pps(), 500);
    BOOST_CHECK_EQUAL (pb->tenants.at(0)->testcase->send_interval(), 2);
};

// Default Testcase if none is set
BOOST_AUTO_TEST_CASE (Testcase_Test2)
{
    Perfbench *pb = new Perfbench;
    char *argv[] = {(char*)"./perfbench",
                    (char*)"--hard-timeout", (char*)"1",
                    NULL};
    int argc = 3;
    pb->argparser->parse(argc, argv);
    pb->start();

    BOOST_CHECK_EQUAL (pb->tenants.at(0)->testcase->name, "FeaturesRequest");
};

// Packetfactory
BOOST_AUTO_TEST_CASE (PacketFactory)
{
    PacketFactory10* pf10 = new PacketFactory10();
    PacketFactory13* pf13 = new PacketFactory13();

    // Test Payload, In_port of PacketIn (OF 1.0)
    char payload[] = { 'H', 'e', 'l', 'l', 'o', '\0' };
    auto pic10 = pf10->packet_in(5, &payload, sizeof(char)*6);
    auto pi10 = dynamic_cast<fluid_msg::of10::PacketIn*>(pic10);
    BOOST_CHECK_EQUAL ((std::string)((char*)(pi10->data())), "Hello");
    BOOST_CHECK_EQUAL (pi10->in_port(), 5);

    // Test Flowmod (OF 1.3)
    auto fmc13 = pf13->flow_mod(5, 7, 1234, 8);
    auto fm13 = dynamic_cast<fluid_msg::of13::FlowMod*>(fmc13);
    BOOST_CHECK_EQUAL (fm13->match().in_port()->value(), 5);
    BOOST_CHECK_EQUAL (fm13->priority(), 1234);
};

// Scheduler
BOOST_AUTO_TEST_CASE (SchedulerTest)
{
    // Normal distribution
    {
        Scheduler* s = new Scheduler("normal", "normal", 8, 9, "");

        std::vector<double> burst_times;
        for (int i = 0; i<10000; i++)
            burst_times.push_back(s->next_inter_burst_time());
        std::vector<int> burst_sizes;
        for (int i = 0; i<10000; i++)
            burst_sizes.push_back(s->next_burst_size());

        double mean_burst_time = std::accumulate( burst_times.begin(), burst_times.end(), 0.0)/burst_times.size();
        double mean_burst_size = std::accumulate( burst_sizes.begin(), burst_sizes.end(), 0.0)/burst_sizes.size();

        BOOST_CHECK (mean_burst_size - 8 <= 0.1);
        BOOST_CHECK (mean_burst_time - 9 <= 0.1);
    }

    // Exponential Distribution
    {
        Scheduler *s = new Scheduler("exponential", "exponential", 8, 9, "");

        std::vector<double> burst_times;
        for (int i = 0; i < 10000; i++)
            burst_times.push_back(s->next_inter_burst_time());
        std::vector<int> burst_sizes;
        for (int i = 0; i < 10000; i++)
            burst_sizes.push_back(s->next_burst_size());

        // Get means
        double mean_burst_time = std::accumulate(burst_times.begin(), burst_times.end(), 0.0) / burst_times.size();
        double mean_burst_size = std::accumulate(burst_sizes.begin(), burst_sizes.end(), 0.0) / burst_sizes.size();

        BOOST_CHECK (mean_burst_size - 8 <= 0.1);
        BOOST_CHECK (mean_burst_time - 9 <= 0.1);
    }
};

BOOST_AUTO_TEST_SUITE_END()


#endif //PERFBENCH_TESTS
