#include "rng.h"

RNGBase::RNGBase(double mean, double stdev) {
    this->mean = mean;
    this->stdev = stdev;
    
    this->seed = std::chrono::system_clock::now().time_since_epoch().count();
    this->generator = std::default_random_engine(seed);
};
RNGBase::~RNGBase() = default;

RNGNormal::RNGNormal(double mean, double stdev) : RNGBase(mean, stdev) {
    // Weibull distr (k, lambda)
    // See wiki for lambda calculation
    double k = 3.602;
    double lambda = mean / (tgamma(1 + 1/k));
    this->distr = std::weibull_distribution<double>(k, lambda);
}
double RNGNormal::draw_random_number() {
    double x = this->distr(this->generator);
    return x;
}



RNGUniform::RNGUniform(double mean, double stdev) : RNGBase(mean, stdev) {}
double RNGUniform::draw_random_number() {
    return this->mean;
}

RNGExponential::RNGExponential(double mean, double stdev) : RNGBase(mean, stdev) {
    this->distr = std::exponential_distribution<double>(1.0/mean);
}
double RNGExponential::draw_random_number() {
    double x = this->distr(this->generator);
    return x;
}

RNGBase* RNGDistrFactory::get_rng(std::string distr, double mean, double stdev) {
	if (distr == "normal") {
		return new RNGNormal(mean, stdev);
	}    
	else if (distr == "exponential") {
		return new RNGExponential(mean, stdev);
	}	
	else {		
		return new RNGUniform(mean, stdev);
	}
};
