#ifndef VEC_H
#define VEC_H

#include <cmath>

class vec3 {
public:
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
    
    vec3 normalized() const {
        double len = length();
        return (len > 0) ? (*this / len) : vec3(0, 0, 0);
    }
    
    void normalize() {
        double len = length();
        if (len > 0) {
            x /= len;
            y /= len;
            z /= len;
        }
    }
    
    double dot(const vec3& v) const { return x * v.x + y * v.y + z * v.z; }
    
    vec3 cross(const vec3& v) const {
        return vec3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
    }
};

inline vec3 operator*(double s, const vec3& v) { return v * s; }

#endif // VEC_H
