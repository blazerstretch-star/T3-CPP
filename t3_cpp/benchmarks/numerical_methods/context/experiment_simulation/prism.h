#ifndef __prisma_h__
#define __prisma_h__

#include "randomgen.h"

class prism {
    
public:
    prism();
    
    void execute();
    void analyze();
    
    //misurati
    double get_A(){return m_A_measured;}
    double get_B(){return m_B_measured;}
    double get_n1(){return m_n1_measured;}
    double get_n2(){return m_n2_measured;}
    double get_th0(){return m_th0_measured;}
    double get_th1(){return m_th1_measured;}
    double get_th2(){return m_th2_measured;}
    double get_delta1(){return m_delta1_measured;}
    double get_delta2(){return m_delta2_measured;}
    
    double get_A_real(){return m_A_real;}
    double get_B_real(){return m_B_real;}
    double get_n1_real(){return m_n1_real;}
    double get_n2_real(){return m_n2_real;}
    double get_th0_real(){return m_th0_real;}
    double get_th1_real(){return m_th1_real;}
    double get_th2_real(){return m_th2_real;}
    double get_delta1_real(){return m_delta1_real;}
    double get_delta2_real(){return m_delta2_real;}
    
private:
    randomgen m_randgen;
    
    double m_lambda1, m_lambda2, m_alpha, m_sigma_theta; //errore strumentale
    
    double m_A_real, m_A_measured;
    double m_B_real, m_B_measured;
    double m_n1_real, m_n1_measured;
    double m_n2_real, m_n2_measured;
    double m_th0_real, m_th0_measured;
    double m_th1_real, m_th1_measured;
    double m_th2_real, m_th2_measured;
    double m_delta1_real, m_delta1_measured;
    double m_delta2_real, m_delta2_measured;

};
#endif
