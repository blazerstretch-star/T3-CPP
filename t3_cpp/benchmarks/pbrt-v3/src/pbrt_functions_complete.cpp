#include "pbrt_functions.h"
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


namespace pbrt {

// Geometry function implementations

template <typename T>
inline T Dot(const Vector3<T> &v1, const Vector3<T> &v2) {
    // FUNCTION_ID: pbrt_func001 - START
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
    // FUNCTION_ID: pbrt_func001 - END
}

template <typename T>
inline Vector3<T> Cross(const Vector3<T> &v1, const Vector3<T> &v2) {
    // FUNCTION_ID: pbrt_func002 - START
    double v1x = v1.x, v1y = v1.y, v1z = v1.z;
    double v2x = v2.x, v2y = v2.y, v2z = v2.z;
    return Vector3<T>((v1y * v2z) - (v1z * v2y), 
                      (v1z * v2x) - (v1x * v2z),
                      (v1x * v2y) - (v1y * v2x));
    // FUNCTION_ID: pbrt_func002 - END
}

template <typename T>
inline Vector3<T> Normalize(const Vector3<T> &v) {
    // FUNCTION_ID: pbrt_func003 - START
    Float len = v.Length();
    return Vector3<T>(v.x / len, v.y / len, v.z / len);
    // FUNCTION_ID: pbrt_func003 - END
}

template <typename T>
inline void CoordinateSystem(const Vector3<T> &v1, Vector3<T> *v2, Vector3<T> *v3) {
    // FUNCTION_ID: pbrt_func004 - START
    if (std::abs(v1.x) > std::abs(v1.y))
        *v2 = Vector3<T>(-v1.z, 0, v1.x) / std::sqrt(v1.x * v1.x + v1.z * v1.z);
    else
        *v2 = Vector3<T>(0, v1.z, -v1.y) / std::sqrt(v1.y * v1.y + v1.z * v1.z);
    *v3 = Cross(v1, *v2);
    // FUNCTION_ID: pbrt_func004 - END
}

template <typename T>
inline Float Distance(const Point3<T> &p1, const Point3<T> &p2) {
    // FUNCTION_ID: pbrt_func005 - START
    Float dx = p1.x - p2.x;
    Float dy = p1.y - p2.y;
    Float dz = p1.z - p2.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
    // FUNCTION_ID: pbrt_func005 - END
}

// Sampling function implementations

Vector3f UniformSampleSphere(const Point2f &u) {
    // FUNCTION_ID: pbrt_func006 - START
    Float z = 1 - 2 * u.x;
    Float r = std::sqrt(std::max((Float)0, (Float)1 - z * z));
    Float phi = 2 * Pi * u.y;
    return Vector3f(r * std::cos(phi), r * std::sin(phi), z);
    // FUNCTION_ID: pbrt_func006 - END
}

Float UniformSpherePdf() {
    // FUNCTION_ID: pbrt_func007 - START
    return Inv4Pi;
    // FUNCTION_ID: pbrt_func007 - END
}

Vector3f UniformSampleHemisphere(const Point2f &u) {
    // FUNCTION_ID: pbrt_func008 - START
    Float z = u.x;
    Float r = std::sqrt(std::max((Float)0, (Float)1. - z * z));
    Float phi = 2 * Pi * u.y;
    return Vector3f(r * std::cos(phi), r * std::sin(phi), z);
    // FUNCTION_ID: pbrt_func008 - END
}

Float UniformHemispherePdf() {
    // FUNCTION_ID: pbrt_func009 - START
    return Inv2Pi;
    // FUNCTION_ID: pbrt_func009 - END
}

Point2f ConcentricSampleDisk(const Point2f &u) {
    // FUNCTION_ID: pbrt_func010 - START
    Point2f uOffset(2.f * u.x - 1, 2.f * u.y - 1);
    
    if (uOffset.x == 0 && uOffset.y == 0) 
        return Point2f(0, 0);
    
    Float theta, r;
    if (std::abs(uOffset.x) > std::abs(uOffset.y)) {
        r = uOffset.x;
        theta = (Pi / 4) * (uOffset.y / uOffset.x);
    } else {
        r = uOffset.y;
        theta = (Pi / 2) - (Pi / 4) * (uOffset.x / uOffset.y);
    }
    return Point2f(r * std::cos(theta), r * std::sin(theta));
    // FUNCTION_ID: pbrt_func010 - END
}

Vector3f CosineSampleHemisphere(const Point2f &u) {
    // FUNCTION_ID: pbrt_func011 - START
    Point2f d = ConcentricSampleDisk(u);
    Float z = std::sqrt(std::max((Float)0, 1 - d.x * d.x - d.y * d.y));
    return Vector3f(d.x, d.y, z);
    // FUNCTION_ID: pbrt_func011 - END
}

Float CosineHemispherePdf(Float cosTheta) {
    // FUNCTION_ID: pbrt_func012 - START
    return cosTheta * InvPi;
    // FUNCTION_ID: pbrt_func012 - END
}

Vector3f SphericalDirection(Float sinTheta, Float cosTheta, Float phi) {
    // FUNCTION_ID: pbrt_func013 - START
    return Vector3f(sinTheta * std::cos(phi), 
                    sinTheta * std::sin(phi),
                    cosTheta);
    // FUNCTION_ID: pbrt_func013 - END
}

// MIS heuristic implementations

Float BalanceHeuristic(int nf, Float fPdf, int ng, Float gPdf) {
    // FUNCTION_ID: pbrt_func014 - START
    return (nf * fPdf) / (nf * fPdf + ng * gPdf);
    // FUNCTION_ID: pbrt_func014 - END
}

Float PowerHeuristic(int nf, Float fPdf, int ng, Float gPdf) {
    // FUNCTION_ID: pbrt_func015 - START
    Float f = nf * fPdf, g = ng * gPdf;
    return (f * f) / (f * f + g * g);
    // FUNCTION_ID: pbrt_func015 - END
}

// Explicit template instantiations
template Float Dot<Float>(const Vector3<Float>&, const Vector3<Float>&);
template Vector3<Float> Cross<Float>(const Vector3<Float>&, const Vector3<Float>&);
template Vector3<Float> Normalize<Float>(const Vector3<Float>&);
template void CoordinateSystem<Float>(const Vector3<Float>&, Vector3<Float>*, Vector3<Float>*);
template Float Distance<Float>(const Point3<Float>&, const Point3<Float>&);

} // namespace pbrt
