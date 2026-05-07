
#ifndef __vect_function_h__
#define __vect_function_h__

#include "myalgebra.h"
#include <cmath>
using namespace std;


class vect_function_base {
public:
    // compute the value of the function at a given position abd time t
    virtual vector<double> eval (const vector<double> &x, double t) const = 0;
};

//==============================================
//            SIMPLE HARMONIC OSCILLATOR
//==============================================

class simple_osc: public vect_function_base {
    /*
        * This class represents a simple armonic oscillator.
        * It is a subclass of vect_function_base, characterized by an angular frequency.
    */
 
public:
    simple_osc(double w) { omega = w;};
    virtual vector<double> eval (const vector <double> &x, double t) const;
    
private:
    double omega;
};

//==============================================
//                   PENDULUM
//==============================================

class pendulum: public vect_function_base {
    /*
        * This class represents a pendulum.
        * It is a subclass of vect_function_base, characterized by a length 
    */
public:
    pendulum(double l) { m_l = l;};
    virtual vector<double> eval (const vector <double> &x, double t) const;
    
private:
    double m_l;
};


//==============================================
//        DRIVEN HARMONIC OSCILLATOR
//==============================================

class oscillator: public vect_function_base {
public:
    oscillator(double omega, double omega_0, double gamma)
    { m_omega_0 = omega_0; m_omega = omega; m_gamma = gamma;};
    
    virtual vector<double> eval (const vector <double> &x, double t) const;
    
    void SetOmega (double omega) {m_omega = omega;}
    double GetOmega () {return m_omega;}
    double GetGamma (){return m_gamma;}

private:
    double m_omega_0;
    double m_gamma;
    double m_omega;

};
#endif // vect_functionbase
