
#ifndef __vector_field_h__
#define __vector_field_h__

#include "position.h"

class vector_field: public position {
/*
    * This class represents a vector field.
    * It is characterized y the x, y, z components of the field.
*/
    
public:
    
    vector_field (const position&);
    vector_field(const position&, double fx, double fy, double fz); 
    vector_field(double x, double y, double z, double fx, double fy, double fz); 
    vector_field(const vector_field& v); 
    ~vector_field(){}

    //overloading operators +
    vector_field& operator+= (const vector_field&); 
    vector_field operator+ (const vector_field&) const; 

    // setters and getters
    double GetFx() const;
    double GetFy() const;
    double GetFz() const;
    void SetFx(double x);
    void SetFy(double y);
    void SetFz(double z);
    
    double module() const; 

private:
    double m_fx, m_fy, m_fz; 

};

#endif

