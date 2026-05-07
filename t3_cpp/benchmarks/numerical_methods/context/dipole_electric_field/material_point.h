#ifndef __material_point_h__
#define __material_point_h__

#include "particle.h"
#include "position.h"
#include "vector_field.h" 

#define EPSILON_0 8.854e-12 // epsilon zero

class material_point: public position, public particle
/*
    * This class represents a material point.
    * It is a subclass of position and particle, with position and charge.
    * It has a method to calculate the electric field at a given position.
*/

{
public:
    
    material_point (): particle(), position(){}
    material_point (const particle& p, const position& v):  position(v), particle(p) {}
    material_point (double mass, double charge, double x, double y, double z): position(x,y,z), particle(mass,charge){}
    ~material_point(){}

    vector_field electric_field(const position& p) const; // electric at a given position

};


#endif

