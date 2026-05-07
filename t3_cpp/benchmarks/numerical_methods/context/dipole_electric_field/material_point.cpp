#include "material_point.h"

vector_field material_point::electric_field(const position& p) const {

    vector_field result (p); 
    double r = Distance (p); 
    double K = m_q/ (4*M_PI*EPSILON_0 * pow(r,3));
    
    result.SetFx (K * (p.GetX() - m_x));
    result.SetFy (K * (p.GetY() - m_y));
    result.SetFz (K * (p.GetZ() - m_z));

    return result;
}


