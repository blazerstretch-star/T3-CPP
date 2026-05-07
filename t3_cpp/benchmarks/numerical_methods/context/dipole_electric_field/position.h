#include <iostream>
#include <cmath>
#include <iomanip>

using namespace std;

#ifndef __position_h__
#define __position_h__

class position {
/*
    * This class represents a position.
    * It characterizes the x, y, z coordinates.
    * It has a method to calculate the distance between two positions.
    * coordinates may be spherical, cartesian or cylindrical.
*/
    
public:
    position(): m_x (0), m_y (0), m_z (0){}
    position(double x, double y, double z) {
        m_x = x;
        m_y = y;
        m_z = z;
    }
    
    //copy constructor
    position(const position &P){
        m_x = P.GetX();
        m_y = P.GetY();
        m_z = P.GetZ();
    }
    ~position(){}

    void SetX(double x);
    void SetY(double y);
    void SetZ(double z);

    //cartesian coordinates
    double GetX() const;
    double GetY() const;
    double GetZ() const;
    
    //spherical coordinates
    double GetRho() const; 
    double GetTheta() const;
    double GetPhi() const;

    // cylindrical coordinates
    double GetR() const; 
    
    //distanza da un altra position inserita
    double Distance(const position& p) const;
    
protected:
    double m_x, m_y, m_z;
};
#endif //__posizione_h__

