#ifndef __diff_eq_base_h__
#define __diff_eq_base_h__

#include "fvect.h"
#include "myalgebra.h"

using namespace std;

class diff_eq_base {
public:
   virtual vector<double> step (const vector<double> &x, double t, double h, vect_function_base* v) const = 0;
    
};

class eulero: public diff_eq_base {
 
public:
    virtual vector<double> step (const vector<double> &x, double t, double h, vect_function_base* v) const;
};

class runge_kutta: public diff_eq_base {

public:
    virtual vector<double> step (const vector<double> &x, double t, double h, vect_function_base* v) const;
};

#endif // __diff_eq_base_h__

