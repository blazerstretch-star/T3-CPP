
#include "equadiff.h"

vector<double> eulero::step (const vector<double> &v, double t, double h, vect_function_base* f) const {
    // FUNCTION_ID: nummethod_func009 - START
    // x  is of the form (x,v) at time t  
    return v + ( h * f->eval (v,t) );
    // FUNCTION_ID: nummethod_func009 - END
}

vector<double> runge_kutta::step (const vector<double> &v, double t, double h, vect_function_base* f) const {
    // FUNCTION_ID: nummethod_func010 - START
    // x  is of the form (x,v) at time t   
    vector<double>  k1 = f->eval ( v,  t );
    vector<double>  k2 = f->eval ( v + k1 * (h/2.), t + h/2.);
    vector<double>  k3 = f->eval ( v + k2 * (h/2.), t + h/2.);
    vector<double>  k4 = f->eval ( v + k3 * h, t + h );
    return v + (k1 + 2. * k2 + 2.* k3 + k4) * (h/6.);
    // FUNCTION_ID: nummethod_func010 - END
}
