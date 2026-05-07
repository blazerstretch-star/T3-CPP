#ifndef MD_TYPES_H
#define MD_TYPES_H

#include <cmath>

// Simple 3D vector structure
struct vec3 {
    double x, y, z;
    
    vec3() : x(0), y(0), z(0) {}
    vec3(double x, double y, double z) : x(x), y(y), z(z) {}
    
    vec3 operator+(const vec3& v) const { return vec3(x + v.x, y + v.y, z + v.z); }
    vec3 operator-(const vec3& v) const { return vec3(x - v.x, y - v.y, z - v.z); }
    vec3 operator*(double s) const { return vec3(x * s, y * s, z * s); }
    vec3 operator/(double s) const { return vec3(x / s, y / s, z / s); }
    
    vec3& operator+=(const vec3& v) { x += v.x; y += v.y; z += v.z; return *this; }
    vec3& operator-=(const vec3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    vec3& operator*=(double s) { x *= s; y *= s; z *= s; return *this; }
    vec3& operator/=(double s) { x /= s; y /= s; z /= s; return *this; }
    
    double length() const { return std::sqrt(x * x + y * y + z * z); }
    double lengthSquared() const { return x * x + y * y + z * z; }
};

inline vec3 operator*(double s, const vec3& v) { return v * s; }

#endif // MD_TYPES_H
