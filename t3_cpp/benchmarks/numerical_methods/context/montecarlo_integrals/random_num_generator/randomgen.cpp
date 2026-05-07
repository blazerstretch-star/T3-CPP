#include "randomgen.h"

double randomgen::rand(){
    // FUNCTION_ID: nummethod_func011 - START
    int n = (m_a * m_seed + m_c) % (m_m); 
    m_seed = static_cast<unsigned int>(n);
    return static_cast<double >(n)/(m_m);
    // FUNCTION_ID: nummethod_func011 - END
};

double randomgen::uniform(double min, double max){
    // FUNCTION_ID: nummethod_func012 - START
    return min + (max-min) * rand(); 
    // FUNCTION_ID: nummethod_func012 - END
};

double randomgen::exp(double lambda){
    // FUNCTION_ID: nummethod_func014 - START
    return -1./(lambda)* log(1-rand());
    // FUNCTION_ID: nummethod_func014 - END
};

double randomgen::gauss(double mean, double sigma){
    // FUNCTION_ID: nummethod_func013 - START
    double s = rand();
    double t = rand();
    double x = sqrt(-2*log(s))*cos(2.* M_PI*t);
    return mean + x * sigma; 
    // FUNCTION_ID: nummethod_func013 - END
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
