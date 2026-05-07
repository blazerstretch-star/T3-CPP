#include "fvect.h"
#define g 9.81

    vector <double> simple_osc::eval (const vector <double> &x, double t) const{
    vector<double> result (x.size()); 
    result[0] = x[1];
    result[1] = - (omega * omega) * x[0];
    return result;
    
    // x contains as a first component the position and as second the velocity x = [pos, vel] 
    // result is a vector (position, velocity) = (v, omega^2 x) in that point
}

    vector<double> pendulum::eval (const vector <double> &x, double t) const{
    vector<double> result(x.size() ); 

       result[0] = x[1];
       result[1] = - ( g / m_l) * sin ( x[0]);
       return result;
    // x contains as a first component theta and as second omega x = [theta, omega] 
    // result is a vector (theta, omega)  in that point

}

    vector <double> oscillator::eval (const vector <double> &x, double t) const{
    vector<double> result (x.size()); 
    
    result[0] = x[1];
    result[1] = - (m_omega_0 * m_omega_0) * x[0] - m_gamma * x[1] + sin(m_omega * t);
    return result;
}

