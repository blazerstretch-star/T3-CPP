#include "position.h"

void position :: SetX(double x){m_x = x;}
void position :: SetY(double y){m_y = y;}
void position :: SetZ(double z){m_z = z;}

//cartesian 
double position :: GetX() const{return m_x;}
double position :: GetY() const{return m_y;}
double position :: GetZ() const{return m_z;}

//spherical coordinates
double position :: GetRho() const{return sqrt (m_x*m_x+m_y*m_y+m_z*m_z);}
double position :: GetPhi() const{return atan2 (m_y,m_x);}                                 
double position :: GetTheta() const{return acos(m_z/GetRho());}

//cylindrical coordinates
double position :: GetR() const{return sqrt (m_x*m_x+m_y*m_y);}

//distance from another position
double position :: Distance(const position& p) const{
    return sqrt( pow(p.GetX()-GetX(),2)
                + pow(p.GetY()-GetY(),2)
                + pow(p.GetZ()-GetZ(),2) );
}
