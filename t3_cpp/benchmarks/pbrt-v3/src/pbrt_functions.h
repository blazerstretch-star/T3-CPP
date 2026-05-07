#pragma once

#include <cmath>
#include <algorithm>

namespace pbrt {

// Type definitions
using Float = float;
constexpr Float Pi = 3.14159265358979323846;
constexpr Float InvPi = 0.31830988618379067154;
constexpr Float Inv2Pi = 0.15915494309189533577;
constexpr Float Inv4Pi = 0.07957747154594766788;

template <typename T>
class Vector3 {
public:
    Vector3() : x(0), y(0), z(0) {}
    Vector3(T xx, T yy, T zz) : x(xx), y(yy), z(zz) {}
    Float Length() const { return std::sqrt(x * x + y * y + z * z); }
    Float LengthSquared() const { return x * x + y * y + z * z; }
    Vector3<T> operator/(T f) const {
        Float inv = (Float)1 / f;
        return Vector3<T>(x * inv, y * inv, z * inv);
    }
    T x, y, z;
};

template <typename T>
class Point2 {
public:
    Point2() : x(0), y(0) {}
    Point2(T xx, T yy) : x(xx), y(yy) {}
    T x, y;
};

template <typename T>
class Point3 {
public:
    Point3() : x(0), y(0), z(0) {}
    Point3(T xx, T yy, T zz) : x(xx), y(yy), z(zz) {}
    T x, y, z;
};

using Vector3f = Vector3<Float>;
using Point2f = Point2<Float>;
using Point3f = Point3<Float>;

// Geometry functions
template <typename T>
inline T Dot(const Vector3<T> &v1, const Vector3<T> &v2);

template <typename T>
inline Vector3<T> Cross(const Vector3<T> &v1, const Vector3<T> &v2);

template <typename T>
inline Vector3<T> Normalize(const Vector3<T> &v);

template <typename T>
inline void CoordinateSystem(const Vector3<T> &v1, Vector3<T> *v2, Vector3<T> *v3);

template <typename T>
inline Float Distance(const Point3<T> &p1, const Point3<T> &p2);

// Sampling functions
Vector3f UniformSampleSphere(const Point2f &u);
Float UniformSpherePdf();
Vector3f UniformSampleHemisphere(const Point2f &u);
Float UniformHemispherePdf();
Point2f ConcentricSampleDisk(const Point2f &u);
Vector3f CosineSampleHemisphere(const Point2f &u);
Float CosineHemispherePdf(Float cosTheta);
Vector3f SphericalDirection(Float sinTheta, Float cosTheta, Float phi);

// MIS heuristics
Float BalanceHeuristic(int nf, Float fPdf, int ng, Float gPdf);
Float PowerHeuristic(int nf, Float fPdf, int ng, Float gPdf);

} // namespace pbrt
