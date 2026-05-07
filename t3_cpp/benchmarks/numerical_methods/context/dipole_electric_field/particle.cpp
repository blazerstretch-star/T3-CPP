#include "particle.h"

particle :: particle(){
    m_mass = 0;
    m_q = 0;
}

 particle :: particle (double mass, double charge){
    m_mass = mass;
    m_q = charge;
}


void  particle :: Print () const{
    cout << "particle: " << endl
         << "Mass: " << m_mass << endl
         << "charge: " << m_q << endl;

}

