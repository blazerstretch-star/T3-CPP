
#ifndef __particle_h__
#define __particle_h__
#include <iostream>
using namespace std;

class particle {
/*
    * This class represents a particle.
    * It characterizes the mass, charge.
*/
public:

    particle(); 
    particle (double mass, double charge); //costruttore argomenti
    ~particle(){}
    
    double GetMass() const {return m_mass;}
    double GetCharge() const{return m_q;}
    void SetMass(double mass){m_mass = mass;}
    void SetCharge (double charge){m_q = charge;}
    void Print () const;
    
protected:
    double m_mass;
    double m_q;
};

#endif
