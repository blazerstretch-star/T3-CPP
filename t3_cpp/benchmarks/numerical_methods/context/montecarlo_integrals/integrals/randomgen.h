
#ifndef __randomgen_h_
#define __randomgen_h_

#include <iostream>
#include <cmath>

class randomgen{
    
public:
    randomgen (unsigned int seed){ 
        m_seed = seed;
        m_a = 1664525;
        m_c = 1013904223;
        m_m =  pow(2,31);
    };
    void SetA(unsigned int a){m_a = a;}
    void SetC(unsigned int c){m_c = c;}
    void SetM(unsigned int m){m_m = m;}

    double rand();
    double uniform(double min, double max);
    double exp(double lambda);
    double gauss(double mean, double sigma);
    double gauss_AR(double mean, double sigma, double a, double b); // accept reject

private:
    
    unsigned int m_a, m_c,m_m;
    unsigned int m_seed;
    
};

#endif //__randomgen_h_
