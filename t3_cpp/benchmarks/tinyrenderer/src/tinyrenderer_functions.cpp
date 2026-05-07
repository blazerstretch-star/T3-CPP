#include "tinyrenderer_functions.h"
#include <algorithm>
#include <cmath>
// Standard C++ Library (pre-included for agent evaluation)
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <queue>
#include <stack>
#include <deque>
#include <list>
#include <algorithm>
#include <cmath>
#include <string>
#include <memory>
#include <utility>


namespace TinyRenderer {

double vec3_dot(const vec3& lhs, const vec3& rhs) {
// FUNCTION_ID: tinyrenderer_func001 - START
    double ret = 0;
    for (int i=3; i--; ret+=lhs[i]*rhs[i]);
    return ret;
// FUNCTION_ID: tinyrenderer_func001 - END
}

vec3 vec3_add(const vec3& lhs, const vec3& rhs) {
// FUNCTION_ID: tinyrenderer_func002 - START
    vec3 ret = lhs;
    for (int i=3; i--; ret[i]+=rhs[i]);
    return ret;
// FUNCTION_ID: tinyrenderer_func002 - END
}

vec3 vec3_subtract(const vec3& lhs, const vec3& rhs) {
// FUNCTION_ID: tinyrenderer_func003 - START
    vec3 ret = lhs;
    for (int i=3; i--; ret[i]-=rhs[i]);
    return ret;
// FUNCTION_ID: tinyrenderer_func003 - END
}

vec3 vec3_scale(const vec3& v, const double scalar) {
// FUNCTION_ID: tinyrenderer_func004 - START
    vec3 ret = v;
    for (int i=3; i--; ret[i]*=scalar);
    return ret;
// FUNCTION_ID: tinyrenderer_func004 - END
}

double vec3_norm(const vec3& v) {
// FUNCTION_ID: tinyrenderer_func005 - START
    double sum = 0;
    for (int i=3; i--; sum += v[i]*v[i]);
    return std::sqrt(sum);
// FUNCTION_ID: tinyrenderer_func005 - END
}

vec3 vec3_normalize(const vec3& v) {
// FUNCTION_ID: tinyrenderer_func006 - START
    double sum = 0;
    for (int i=3; i--; sum += v[i]*v[i]);
    double norm = std::sqrt(sum);
    vec3 ret = v;
    for (int i=3; i--; ret[i] /= norm);
    return ret;
// FUNCTION_ID: tinyrenderer_func006 - END
}

vec3 vec3_cross(const vec3& v1, const vec3& v2) {
// FUNCTION_ID: tinyrenderer_func007 - START
    vec3 result;
    result.x = v1.y*v2.z - v1.z*v2.y;
    result.y = v1.z*v2.x - v1.x*v2.z;
    result.z = v1.x*v2.y - v1.y*v2.x;
    return result;
// FUNCTION_ID: tinyrenderer_func007 - END
}

mat3 mat3_transpose(const mat3& m) {
// FUNCTION_ID: tinyrenderer_func008 - START
    mat3 ret;
    for (int i=3; i--; )
        for (int j=3; j--; ret[i][j]=m[j][i]);
    return ret;
// FUNCTION_ID: tinyrenderer_func008 - END
}

mat3 mat3_multiply(const mat3& lhs, const mat3& rhs) {
// FUNCTION_ID: tinyrenderer_func009 - START
    mat3 result;
    for (int i=3; i--; )
        for (int j=3; j--; )
            for (int k=3; k--; result[i][j]+=lhs[i][k]*rhs[k][j]);
    return result;
// FUNCTION_ID: tinyrenderer_func009 - END
}

vec3 mat3_vec3_multiply(const mat3& m, const vec3& v) {
// FUNCTION_ID: tinyrenderer_func010 - START
    vec3 ret;
    for (int i=3; i--; ) {
        ret[i] = 0;
        for (int j=3; j--; ret[i]+=m[i][j]*v[j]);
    }
    return ret;
// FUNCTION_ID: tinyrenderer_func010 - END
}

double mat3_determinant(const mat3& m) {
// FUNCTION_ID: tinyrenderer_func011 - START
    double ret = 0;
    for (int i=3; i--; ) {
        mat<2,2> submatrix;
        for (int j=2; j--; )
            for (int k=2; k--; submatrix[j][k]=m[j+int(j>=0)][k+int(k>=i)]);
        double det = submatrix[0][0]*submatrix[1][1] - submatrix[0][1]*submatrix[1][0];
        ret += m[0][i] * det * ((i)%2 ? -1 : 1);
    }
    return ret;
// FUNCTION_ID: tinyrenderer_func011 - END
}

double mat3_cofactor(const mat3& m, const int row, const int col) {
// FUNCTION_ID: tinyrenderer_func012 - START
    mat<2,2> submatrix;
    for (int i=2; i--; )
        for (int j=2; j--; submatrix[i][j]=m[i+int(i>=row)][j+int(j>=col)]);
    double det = submatrix[0][0]*submatrix[1][1] - submatrix[0][1]*submatrix[1][0];
    return det * ((row+col)%2 ? -1 : 1);
// FUNCTION_ID: tinyrenderer_func012 - END
}

mat3 mat3_invert(const mat3& m) {
// FUNCTION_ID: tinyrenderer_func013 - START
    mat3 adjugate_transpose;
    for (int i=3; i--; )
        for (int j=3; j--; ) {
            mat<2,2> submatrix;
            for (int k=2; k--; )
                for (int l=2; l--; submatrix[k][l]=m[k+int(k>=i)][l+int(l>=j)]);
            double det = submatrix[0][0]*submatrix[1][1] - submatrix[0][1]*submatrix[1][0];
            adjugate_transpose[i][j] = det * ((i+j)%2 ? -1 : 1);
        }
    
    double det = 0;
    for (int i=3; i--; det += adjugate_transpose[0][i] * m[0][i]);
    
    mat3 result;
    for (int i=3; i--; )
        for (int j=3; j--; result[i][j]=adjugate_transpose[i][j]/det);
    return result;
// FUNCTION_ID: tinyrenderer_func013 - END
}

mat4 mat4_transpose(const mat4& m) {
// FUNCTION_ID: tinyrenderer_func014 - START
    mat4 ret;
    for (int i=4; i--; )
        for (int j=4; j--; ret[i][j]=m[j][i]);
    return ret;
// FUNCTION_ID: tinyrenderer_func014 - END
}

mat4 mat4_multiply(const mat4& lhs, const mat4& rhs) {
// FUNCTION_ID: tinyrenderer_func015 - START
    mat4 result;
    for (int i=4; i--; )
        for (int j=4; j--; )
            for (int k=4; k--; result[i][j]+=lhs[i][k]*rhs[k][j]);
    return result;
// FUNCTION_ID: tinyrenderer_func015 - END
}

vec4 mat4_vec4_multiply(const mat4& m, const vec4& v) {
// FUNCTION_ID: tinyrenderer_func016 - START
    vec4 ret;
    for (int i=4; i--; ) {
        ret[i] = 0;
        for (int j=4; j--; ret[i]+=m[i][j]*v[j]);
    }
    return ret;
// FUNCTION_ID: tinyrenderer_func016 - END
}

mat4 lookat(const vec3& eye, const vec3& center, const vec3& up) {
// FUNCTION_ID: tinyrenderer_func017 - START
    vec3 n_unnorm;
    for (int i=3; i--; n_unnorm[i] = eye[i] - center[i]);
    double n_sum = 0;
    for (int i=3; i--; n_sum += n_unnorm[i]*n_unnorm[i]);
    double n_norm = std::sqrt(n_sum);
    vec3 n;
    for (int i=3; i--; n[i] = n_unnorm[i] / n_norm);
    
    vec3 l_unnorm;
    l_unnorm.x = up.y*n.z - up.z*n.y;
    l_unnorm.y = up.z*n.x - up.x*n.z;
    l_unnorm.z = up.x*n.y - up.y*n.x;
    double l_sum = 0;
    for (int i=3; i--; l_sum += l_unnorm[i]*l_unnorm[i]);
    double l_norm = std::sqrt(l_sum);
    vec3 l;
    for (int i=3; i--; l[i] = l_unnorm[i] / l_norm);
    
    vec3 m_unnorm;
    m_unnorm.x = n.y*l.z - n.z*l.y;
    m_unnorm.y = n.z*l.x - n.x*l.z;
    m_unnorm.z = n.x*l.y - n.y*l.x;
    double m_sum = 0;
    for (int i=3; i--; m_sum += m_unnorm[i]*m_unnorm[i]);
    double m_norm = std::sqrt(m_sum);
    vec3 m;
    for (int i=3; i--; m[i] = m_unnorm[i] / m_norm);
    
    mat4 orientation = {{
        {l.x, l.y, l.z, 0},
        {m.x, m.y, m.z, 0},
        {n.x, n.y, n.z, 0},
        {0, 0, 0, 1}
    }};
    
    mat4 translation = {{
        {1, 0, 0, -center.x},
        {0, 1, 0, -center.y},
        {0, 0, 1, -center.z},
        {0, 0, 0, 1}
    }};
    
    mat4 result;
    for (int i=4; i--; )
        for (int j=4; j--; )
            for (int k=4; k--; result[i][j]+=orientation[i][k]*translation[k][j]);
    return result;
// FUNCTION_ID: tinyrenderer_func017 - END
}

mat4 perspective(const double fov) {
// FUNCTION_ID: tinyrenderer_func018 - START
    mat4 result = {{
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, -1.0/fov, 1}
    }};
    return result;
// FUNCTION_ID: tinyrenderer_func018 - END
}

mat4 viewport(const int x, const int y, const int w, const int h) {
// FUNCTION_ID: tinyrenderer_func019 - START
    mat4 result = {{
        {w/2.0, 0, 0, x+w/2.0},
        {0, h/2.0, 0, y+h/2.0},
        {0, 0, 1, 0},
        {0, 0, 0, 1}
    }};
    return result;
// FUNCTION_ID: tinyrenderer_func019 - END
}

vec3 barycentric(const vec2& p, const vec2& a, const vec2& b, const vec2& c) {
// FUNCTION_ID: tinyrenderer_func020 - START
    mat3 ABC = {{
        {a.x, a.y, 1.0},
        {b.x, b.y, 1.0},
        {c.x, c.y, 1.0}
    }};
    mat3 adjugate_transpose;
    for (int i=3; i--; )
        for (int j=3; j--; ) {
            mat<2,2> submatrix;
            for (int k=2; k--; )
                for (int l=2; l--; submatrix[k][l]=ABC[k+int(k>=i)][l+int(l>=j)]);
            double det = submatrix[0][0]*submatrix[1][1] - submatrix[0][1]*submatrix[1][0];
            adjugate_transpose[i][j] = det * ((i+j)%2 ? -1 : 1);
        }
    
    double det = 0;
    for (int i=3; i--; det += adjugate_transpose[0][i] * ABC[0][i]);
    
    mat3 inv_transpose;
    for (int i=3; i--; )
        for (int j=3; j--; inv_transpose[i][j]=adjugate_transpose[i][j]/det);
    
    vec3 point = {p.x, p.y, 1.0};
    vec3 ret;
    for (int i=3; i--; ) {
        ret[i] = 0;
        for (int j=3; j--; ret[i]+=inv_transpose[i][j]*point[j]);
    }
    return ret;
// FUNCTION_ID: tinyrenderer_func020 - END
}

bool is_backface(const vec2& a, const vec2& b, const vec2& c) {
// FUNCTION_ID: tinyrenderer_func021 - START
    mat3 ABC = {{
        {a.x, a.y, 1.0},
        {b.x, b.y, 1.0},
        {c.x, c.y, 1.0}
    }};
    double det = 0;
    for (int i=3; i--; ) {
        mat<2,2> submatrix;
        for (int j=2; j--; )
            for (int k=2; k--; submatrix[j][k]=ABC[j+int(j>=0)][k+int(k>=i)]);
        double subdet = submatrix[0][0]*submatrix[1][1] - submatrix[0][1]*submatrix[1][0];
        det += ABC[0][i] * subdet * ((i)%2 ? -1 : 1);
    }
    return det < 1;
// FUNCTION_ID: tinyrenderer_func021 - END
}

} // namespace TinyRenderer
