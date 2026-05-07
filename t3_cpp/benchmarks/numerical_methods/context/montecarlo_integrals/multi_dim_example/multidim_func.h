
#ifndef __func_Rn_h__
#define __func_Rn_h__

#include <iostream>
#include <cstdlib>
#include <cmath>
#include <vector>

using namespace std;

class funcbase_Rn {
public:
	virtual double eval (const vector<double>& x) const = 0;
};

class f1: public funcbase_Rn {
public:
    virtual double eval (const vector<double>& x) const;
};


#endif
