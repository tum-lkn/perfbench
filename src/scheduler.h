#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "rng.h"

#include <string>
#include <vector>
#include <random>
#include <math.h>
#include <fstream>
#include <sstream> 


class Scheduler {
    int num_samples = 10000;
    unsigned _i_btime = 0;
    unsigned _i_bsize = 0;
    std::vector<double> inter_burst_times;
    std::vector<int> burst_sizes;
    
    RNGBase* bsize_rng;    
    RNGBase* btime_rng;    
    
	public:
        std::string bsize_distr;
        std::string btime_distr;
        double mean_inter_burst_time;
        int mean_burst_size;
        std::string scheduling_file;
        
    Scheduler(std::string bsize_distr, std::string btime_distr, int mean_burst_size, double mean_inter_burst_time, std::string scheduling_file);
	~Scheduler();

    void populate_data();    
    void parse_file(std::string f);    
    void generate_samples(int n);    
    double next_inter_burst_time();
    int next_burst_size();
};


#endif
#pragma once