#include <iostream>
#include <cstdlib>
#include <cmath>
using namespace std;

#ifndef __func_h__
#define __func_h__

class funcbase{
public:
	virtual double eval (double x) const = 0;
};

class fsin: public funcbase {
public:
	virtual double eval (double x) const;
};

class f1: public funcbase {
public:
    virtual double eval (double x) const{
        return exp(x)+exp(-x);
    }
};

class poldeg2: public funcbase {
public:
	poldeg2();
	poldeg2(double a, double b, double c);
	~poldeg2();

	virtual double eval (double x) const;

	void setparameters (double _a, double _b, double _c);
	double getparameter_a () const;
	double getparameter_b () const;
	double getparameter_c () const;

private:
	double m_a;
	double m_b;
	double m_c;
};

class f_tan: public funcbase { 
public:
    virtual double eval (double x) const;
};



#endif
