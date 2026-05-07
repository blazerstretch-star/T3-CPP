#include "prism.h"
#include <cmath>

prism::prism():
m_randgen(1),
m_lambda1(579.1E-9),
m_lambda2(404.7E-9),
m_alpha(60. * M_PI/180.),
m_sigma_theta(0.3E-3),
m_A_real(2.7),
m_B_real(60000E-18)
{
    m_n1_real = sqrt(m_A_real + m_B_real / pow(m_lambda1,2) );
    m_n2_real = sqrt(m_A_real + m_B_real / pow(m_lambda2,2) );
    
    m_th0_real = M_PI/2.;
    
    m_th1_real = m_th0_real + 2. * asin(m_n1_real * sin(0.5 * m_alpha)) - m_alpha;
    m_th2_real = m_th0_real + 2. * asin(m_n2_real * sin(0.5 * m_alpha)) - m_alpha;
    m_delta1_real = m_th1_real -  m_th0_real;
    m_delta2_real = m_th2_real -  m_th0_real;
}

void prism::execute(){
    m_th0_measured = m_randgen.gauss(m_th0_real, m_sigma_theta);
    m_th1_measured = m_randgen.gauss(m_th1_real, m_sigma_theta);
    m_th2_measured = m_randgen.gauss(m_th2_real, m_sigma_theta);

}

void prism::analyze(){
    m_delta1_measured = m_th1_measured - m_th0_measured;
    m_delta2_measured = m_th2_measured - m_th0_measured;

    m_n1_measured = sin((m_delta1_measured+m_alpha)/2.) / sin(m_alpha/2.);
    m_n2_measured = sin((m_delta2_measured+m_alpha)/2.) / sin(m_alpha/2.);

    m_A_measured = ( pow(m_lambda2,2) * pow(m_n2_measured,2) - pow(m_lambda1,2)* pow(m_n1_measured,2) )/( pow(m_lambda2,2) - pow(m_lambda1,2) );
    
    m_B_measured = ( pow(m_n2_measured,2) - pow(m_n1_measured,2)  )
    / ( 1./pow(m_lambda2,2) - 1./pow(m_lambda1,2) );  
}
