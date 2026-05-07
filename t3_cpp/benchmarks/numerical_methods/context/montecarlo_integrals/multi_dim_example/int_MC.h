#ifndef __integral_mc_h__
#define __integral_mc_h__

#include "randomgen.h"
#include "func.h"
#include "multidim_func.h"

#include <vector>

class integral_mc {
public:
    integral_mc (unsigned int seed){m_rand = new randomgen(seed);};

	double int_mean (double a, double b, funcbase* f, int n_points);
	double int_HoM (double a, double b, double fmax, funcbase* f, int punti);
    double int_mean (vector<double>& a, vector<double>& b, funcbase_Rn* f, int n_points); // multidim
   
private: 
    randomgen* m_rand; 
};

#endif
