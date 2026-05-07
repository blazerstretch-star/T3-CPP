#include "randomgen.h"

double randomgen::rand(){
    
    int n = (m_a * m_seed + m_c) % (m_m); 
    m_seed = static_cast<unsigned int>(n);
    return static_cast<double >(n)/(m_m);
};

double randomgen::uniform(double min, double max){
    return min + (max-min) * rand(); 
};

double randomgen::exp(double lambda){
    return -1./(lambda)* log(1-rand());
};

double randomgen::gauss(double mean, double sigma){
    double s = rand();
    double t = rand();
    double x = sqrt(-2*log(s))*cos(2.* M_PI*t);
    return mean + x * sigma; 
};

double randomgen::gauss_AR (double mean, double sigma, double a, double b){
    bool flag = true;
    double y,x,f;
    while (flag){
        y = (1./(sigma * sqrt(2*M_PI))) * rand(); 
        x =  a + (b-a) * rand(); 
        f = (1./(sigma * sqrt(2*M_PI)))* exp( -pow((x-mean),2) /(2*sigma*sigma)); 
        if(f>y) flag = false; 
}
    return x;
};
