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


#endif
