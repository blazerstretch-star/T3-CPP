#ifndef __integral_mc_h__
#define __integral_mc_h__

#include "randomgen.h"
#include "func.h"

class integral_mc {
public:
    integral_mc (unsigned int seed){m_rand = new randomgen(seed);};

	double int_mean (double a, double b, funcbase* f, int n_points);
	double int_HoM (double a, double b, double fmax, funcbase* f, int punti);
    
private: 
    randomgen* m_rand; 
};

#endif
