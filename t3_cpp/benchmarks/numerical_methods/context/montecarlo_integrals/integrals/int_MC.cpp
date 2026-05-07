#include "int_MC.h"

double integral_mc::int_mean (double a, double b, funcbase* f, int n_points){
    // FUNCTION_ID: nummethod_func015 - START
    double sum = 0.;
    for(int i = 0; i< n_points; i++){
        sum += f->eval(m_rand->uniform(a,b));
    }
    return (fabs(b-a)*sum)/n_points;
    // FUNCTION_ID: nummethod_func015 - END
}

double integral_mc::int_HoM (double a, double b, double fmax, funcbase* f, int n_points){
    // FUNCTION_ID: nummethod_func016 - START
    int n_hit = 0;
    int n_tot = 0;
    double y = 0.;
    double x = 0.;
    double f_x = 0.;

    for(int i = 0; i< n_points; i++){
        n_tot ++;
        y = m_rand->uniform(0,fmax);
        x = m_rand->uniform(a,b);
        f_x = f->eval(x);
        if (f_x > y) n_hit ++;
    }
    return fabs(b-a) * fmax * (static_cast <double> (n_hit)/static_cast <double>(n_tot));
    // FUNCTION_ID: nummethod_func016 - END
    
}
