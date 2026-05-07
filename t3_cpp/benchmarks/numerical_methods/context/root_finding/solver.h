
#ifndef __solver_h__
#define __solver_h__

#include <iostream>
#include "func.h"
#include <cmath>

class solver {
public:
	virtual double root_finder (double a, double b, const funcbase* f) = 0;
    
	void set_precision (double epsilon) {m_prec = epsilon;}
    double get_precision () const {return m_prec;}

	void set_a (double a){m_a = a;}
	void set_b (double b){m_b = b;}
    
private:
	double m_a, m_b, m_prec;
	const funcbase* m_f;
};


class bisection: public solver {
public:
	bisection(unsigned int it, double precision) {
        n_iter = it;
        set_precision (precision);
    }
    
    void SetN (unsigned int);
	virtual double root_finder(double a, double b, const funcbase* f);
    
protected:
	unsigned int n_iter;
};


int sign (double);

#endif
