#ifndef RNG_H
#define RNG_H

#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <math.h>
#include <chrono>

/**
 * Baseclass for the RNGs
 */
class RNGBase {
    public:
        long seed;
        std::default_random_engine generator;
        double mean;
		double stdev;
        
    RNGBase(double mean, double stdev);
	~RNGBase();
    
    virtual double draw_random_number() = 0;    
    
};

/**
 * RNG with normal distribution (actually using the weibull distr)
 */
class RNGNormal : public RNGBase {
	std::weibull_distribution<double> distr;
	public:
		RNGNormal(double mean, double stdev);
		double draw_random_number();
};

/**
 * RNG with uniform distribution
 * Always returning the mean
 */
class RNGUniform : public RNGBase {
	public:
		RNGUniform(double mean, double stdev);
		double draw_random_number();
};

/**
 * RNG with exponential distribution
 */
class RNGExponential : public RNGBase {
	std::exponential_distribution<double> distr;
	public:
		RNGExponential(double mean, double stdev);
		double draw_random_number();
};

/**
 * RNG factory
 */
class RNGDistrFactory {    
	public:
		static RNGBase* get_rng(std::string distr, double mean, double stdev);
    
};


#endif
#pragma once