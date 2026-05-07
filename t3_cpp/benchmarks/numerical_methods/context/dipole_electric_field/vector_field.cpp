#include "vector_field.h"

vector_field :: vector_field (const position& p): //costruttore default
    position(p.GetX(),p.GetY(),p.GetZ())
{
    m_fx = 0;
    m_fy = 0;
    m_fz = 0;
}

 vector_field :: vector_field(const position& p, double fx, double fy, double fz): //const con arg e posizione
    position(p.GetX(),p.GetY(),p.GetZ())
{
    m_fx = fx;
    m_fy = fy;
    m_fz = fz;
}

 vector_field :: vector_field(double x, double y, double z, double fx, double fy, double fz): //const con arg senza posizione
    position(x,y,z)
{
    m_fx = fx;
    m_fy = fy;
    m_fz = fz;
}
 vector_field :: vector_field(const vector_field& v): //const con arg senza posizione
    position(v.GetX(),v.GetY(),v.GetZ())
{
    m_fx = v.GetFx();
    m_fy = v.GetFy();
    m_fz = v.GetFz();
}
 
double vector_field :: GetFx() const{return m_fx;}
double vector_field :: GetFy() const{return m_fy;}
double vector_field :: GetFz() const{return m_fz;}
void vector_field :: SetFx(double x){m_fx = x;}
void vector_field :: SetFy(double y){m_fy = y;}
void vector_field :: SetFz(double z){m_fz = z;}
 
 vector_field& vector_field :: operator+= (const vector_field& v)
{
        m_fx+=v.GetFx();
        m_fy+=v.GetFy();
        m_fz+=v.GetFz();

    return *this;
}


 vector_field vector_field :: operator+ (const vector_field& v) const
{
    vector_field sum (v); 
    
    sum.m_fx += GetFx();
    sum.m_fy += GetFy();
    sum.m_fz += GetFz();

    return sum; 
}


double vector_field :: module() const {return sqrt(m_fx*m_fx+m_fy*m_fy+m_fz*m_fz);}

