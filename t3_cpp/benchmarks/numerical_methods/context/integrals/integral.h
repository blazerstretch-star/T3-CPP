#ifndef __integral_h__
#define __integral_h__

#include "func.h"

class integral {
public:
	integral (double a, double b, const funcbase* f);
    integral (double a, double b, const funcbase* f, double prec);


	double midpoint (int N);
    double midpoint(); 
	double simpson (int N);
    double simpson ();  
    double trapezoidal_not_optimized(); 
    double trapezoidal (int N); 
    double trapezoidal (); 

	double Geth(int N) const{ return ( abs(m_b-m_a) )/N;};
    
private: 
	double m_a, m_b; 
	double m_int; 
    double m_old; 

	int m_sign; 
	double m_h; 
    double precision; 
	const funcbase* m_f; 

};

#endif
