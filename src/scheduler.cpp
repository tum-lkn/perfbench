#include "scheduler.h"

Scheduler::Scheduler(std::string bsize_distr, std::string btime_distr, int mean_burst_size, double mean_inter_burst_time, std::string scheduling_file)
{
    this->bsize_distr = bsize_distr;
    this->btime_distr = btime_distr;
    this->mean_burst_size = mean_burst_size;
    this->mean_inter_burst_time = mean_inter_burst_time;    
    this->scheduling_file = scheduling_file;

    this->bsize_rng = RNGDistrFactory::get_rng(bsize_distr, mean_burst_size, 1);
    this->btime_rng = RNGDistrFactory::get_rng(btime_distr, mean_inter_burst_time, 1);

    this->populate_data();
    
    
    
    // DEBUG
    /*
    {
        for ( auto i = this->inter_burst_times.begin(); i != this->inter_burst_times.end(); i++ ) {
            std::cout << *i << ",";
        }
        double sum = std::accumulate(this->inter_burst_times.begin(), this->inter_burst_times.end(), 0.0);
        double mean = sum / this->inter_burst_times.size();
        std::cout << "Mean InterBurstTime: " << mean << "\n";
    }

    {
        for ( auto i = this->burst_sizes.begin(); i != this->burst_sizes.end(); i++ ) {
            std::cout << *i << ",";
        }
        double sum = std::accumulate(this->burst_sizes.begin(), this->burst_sizes.end(), 0.0);
        double mean = sum / this->burst_sizes.size();
        std::cout << "Mean BurstSize: " << mean << "\n";
    }
    */
}

void Scheduler::populate_data()
{
    // No Scheduling File set: Generate RNG Samples
    if(this->scheduling_file.empty()){    
        this->generate_samples(this->num_samples); 
    } 
    else {
        this->parse_file(this->scheduling_file);
    }  
}

void Scheduler::parse_file(std::string f){
    std::ifstream file(f);

    double ibt;
    int bs;

    while (file >> ibt >> bs)
    {
        this->burst_sizes.push_back(bs);     
        this->inter_burst_times.push_back(ibt);
    }
}

void Scheduler::generate_samples(int n) {
    for (int i = 0; i < n; i++) {    
        this->burst_sizes.push_back((int)round(this->bsize_rng->draw_random_number()));     
        this->inter_burst_times.push_back(this->btime_rng->draw_random_number());        
    }
}

double Scheduler::next_inter_burst_time() {
    this->_i_btime++;
    if(this->_i_btime+1 > this->inter_burst_times.size()){ this->_i_btime = 0; }    
    double x = this->inter_burst_times.at(this->_i_btime);
    return x;
}
int Scheduler::next_burst_size() {
    this->_i_bsize++;
    if(this->_i_bsize+1 > this->burst_sizes.size()){ this->_i_bsize = 0; }    
    int x = this->burst_sizes.at(this->_i_bsize);
    return x;
}
