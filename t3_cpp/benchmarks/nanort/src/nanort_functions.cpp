#include "nanort_functions.h"
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


namespace nanort {

// Vector math functions

template <typename T>
real3<T> vneg(const real3<T> &rhs) {
    // FUNCTION_ID: nanort_func001 - START
    return real3<T>(-rhs.x(), -rhs.y(), -rhs.z());
    // FUNCTION_ID: nanort_func001 - END
}

template <typename T>
real3<T> operator*(T f, const real3<T> &v) {
    // FUNCTION_ID: nanort_func002 - START
    return real3<T>(v.x() * f, v.y() * f, v.z() * f);
    // FUNCTION_ID: nanort_func002 - END
}

template <typename T>
T vlength(const real3<T> &rhs) {
    // FUNCTION_ID: nanort_func003 - START
    return std::sqrt(rhs.x() * rhs.x() + rhs.y() * rhs.y() + rhs.z() * rhs.z());
    // FUNCTION_ID: nanort_func003 - END
}

template <typename T>
real3<T> vnormalize(const real3<T> &rhs) {
    // FUNCTION_ID: nanort_func004 - START
    real3<T> v = rhs;
    T len = vlength(rhs);
    if (std::fabs(len) > std::numeric_limits<T>::epsilon()) {
        T inv_len = static_cast<T>(1.0) / len;
        v.v[0] *= inv_len;
        v.v[1] *= inv_len;
        v.v[2] *= inv_len;
    }
    return v;
    // FUNCTION_ID: nanort_func004 - END
}

template <typename T>
real3<T> vcross(const real3<T> a, const real3<T> b) {
    // FUNCTION_ID: nanort_func005 - START
    real3<T> c;
    c[0] = a[1] * b[2] - a[2] * b[1];
    c[1] = a[2] * b[0] - a[0] * b[2];
    c[2] = a[0] * b[1] - a[1] * b[0];
    return c;
    // FUNCTION_ID: nanort_func005 - END
}

template <typename T>
T vdot(const real3<T> a, const real3<T> b) {
    // FUNCTION_ID: nanort_func006 - START
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
    // FUNCTION_ID: nanort_func006 - END
}

template <typename T>
real3<T> vsafe_inverse(const real3<T> v) {
    // FUNCTION_ID: nanort_func007 - START
    real3<T> r;
    
    if (std::fabs(v[0]) < std::numeric_limits<T>::epsilon()) {
        T sgn = (v[0] < static_cast<T>(0)) ? static_cast<T>(-1) : static_cast<T>(1);
        r[0] = std::numeric_limits<T>::infinity() * sgn;
    } else {
        r[0] = static_cast<T>(1.0) / v[0];
    }
    
    if (std::fabs(v[1]) < std::numeric_limits<T>::epsilon()) {
        T sgn = (v[1] < static_cast<T>(0)) ? static_cast<T>(-1) : static_cast<T>(1);
        r[1] = std::numeric_limits<T>::infinity() * sgn;
    } else {
        r[1] = static_cast<T>(1.0) / v[1];
    }
    
    if (std::fabs(v[2]) < std::numeric_limits<T>::epsilon()) {
        T sgn = (v[2] < static_cast<T>(0)) ? static_cast<T>(-1) : static_cast<T>(1);
        r[2] = std::numeric_limits<T>::infinity() * sgn;
    } else {
        r[2] = static_cast<T>(1.0) / v[2];
    }
    
    return r;
    // FUNCTION_ID: nanort_func007 - END
}

// Bounding box functions

template <typename T>
T CalculateSurfaceArea(const real3<T> &min, const real3<T> &max) {
    // FUNCTION_ID: nanort_func008 - START
    real3<T> box = max - min;
    return static_cast<T>(2.0) * (box[0] * box[1] + box[1] * box[2] + box[2] * box[0]);
    // FUNCTION_ID: nanort_func008 - END
}

template <typename T>
void GetBoundingBoxOfTriangle(real3<T> *bmin, real3<T> *bmax, const T *vertices, const unsigned int *faces, unsigned int index) {
    // FUNCTION_ID: nanort_func009 - START
    unsigned int f0 = faces[3 * index + 0];
    unsigned int f1 = faces[3 * index + 1];
    unsigned int f2 = faces[3 * index + 2];
    
    real3<T> p[3];
    p[0] = real3<T>(&vertices[3 * f0]);
    p[1] = real3<T>(&vertices[3 * f1]);
    p[2] = real3<T>(&vertices[3 * f2]);
    
    (*bmin) = p[0];
    (*bmax) = p[0];
    
    for (int i = 1; i < 3; i++) {
        (*bmin)[0] = std::min((*bmin)[0], p[i][0]);
        (*bmin)[1] = std::min((*bmin)[1], p[i][1]);
        (*bmin)[2] = std::min((*bmin)[2], p[i][2]);
        
        (*bmax)[0] = std::max((*bmax)[0], p[i][0]);
        (*bmax)[1] = std::max((*bmax)[1], p[i][1]);
        (*bmax)[2] = std::max((*bmax)[2], p[i][2]);
    }
    // FUNCTION_ID: nanort_func009 - END
}

// NaN-safe comparisons

template <class T>
const T &safemin(const T &a, const T &b) {
    // FUNCTION_ID: nanort_func010 - START
    return (a < b) ? a : b;
    // FUNCTION_ID: nanort_func010 - END
}

template <class T>
const T &safemax(const T &a, const T &b) {
    // FUNCTION_ID: nanort_func011 - START
    return (a > b) ? a : b;
    // FUNCTION_ID: nanort_func011 - END
}

// Ray-AABB intersection (float specialization)

template <>
bool IntersectRayAABB<float>(float *tminOut, float *tmaxOut, float min_t, float max_t, const float bmin[3], const float bmax[3], real3<float> ray_org, real3<float> ray_inv_dir, int ray_dir_sign[3]) {
    // FUNCTION_ID: nanort_func012 - START
    float tmin, tmax;
    
    const float min_x = ray_dir_sign[0] ? bmax[0] : bmin[0];
    const float min_y = ray_dir_sign[1] ? bmax[1] : bmin[1];
    const float min_z = ray_dir_sign[2] ? bmax[2] : bmin[2];
    const float max_x = ray_dir_sign[0] ? bmin[0] : bmax[0];
    const float max_y = ray_dir_sign[1] ? bmin[1] : bmax[1];
    const float max_z = ray_dir_sign[2] ? bmin[2] : bmax[2];
    
    const float tmin_x = (min_x - ray_org[0]) * ray_inv_dir[0];
    const float tmax_x = (max_x - ray_org[0]) * ray_inv_dir[0] * 1.00000024f;
    
    const float tmin_y = (min_y - ray_org[1]) * ray_inv_dir[1];
    const float tmax_y = (max_y - ray_org[1]) * ray_inv_dir[1] * 1.00000024f;
    
    const float tmin_z = (min_z - ray_org[2]) * ray_inv_dir[2];
    const float tmax_z = (max_z - ray_org[2]) * ray_inv_dir[2] * 1.00000024f;
    
    tmin = safemax(tmin_z, safemax(tmin_y, safemax(tmin_x, min_t)));
    tmax = safemin(tmax_z, safemin(tmax_y, safemin(tmax_x, max_t)));
    
    if (tmin <= tmax) {
        (*tminOut) = tmin;
        (*tmaxOut) = tmax;
        return true;
    }
    return false;
    // FUNCTION_ID: nanort_func012 - END
}

// SAH calculation

template <typename T>
T SAH(size_t ns1, T leftArea, size_t ns2, T rightArea, T invS, T Taabb, T Ttri) {
    // FUNCTION_ID: nanort_func013 - START
    T sah;
    sah = static_cast<T>(2.0) * Taabb +
          (leftArea * invS) * static_cast<T>(ns1) * Ttri +
          (rightArea * invS) * static_cast<T>(ns2) * Ttri;
    return sah;
    // FUNCTION_ID: nanort_func013 - END
}

// BVH construction

template <typename T>
bool FindCutFromBinBuffer(T *cut_pos, int *minCostAxis, BinBuffer<T> *bins, const real3<T> &bmin, const real3<T> &bmax) {
    // FUNCTION_ID: nanort_func014 - START
    T minCost[3];
    for (int j = 0; j < 3; ++j) {
        minCost[j] = std::numeric_limits<T>::max();
        
        // Sweep left to accumulate bounding boxes and compute the right-hand side of the cost
        size_t count = 0;
        BBox<T> accumulated_bbox;
        for (size_t i = bins->bin_size - 1; i > 0; --i) {
            Bin<T>& bin = bins->bin[static_cast<unsigned int>(j) * bins->bin_size + i];
            for (int k = 0; k < 3; ++k) {
                accumulated_bbox.bmin[k] = std::min(bin.bbox.bmin[k], accumulated_bbox.bmin[k]);
                accumulated_bbox.bmax[k] = std::max(bin.bbox.bmax[k], accumulated_bbox.bmax[k]);
            }
            count += bin.count;
            bin.cost = T(count) * CalculateSurfaceArea(accumulated_bbox.bmin, accumulated_bbox.bmax);
        }
        
        // Sweep right to compute the full cost
        count = 0;
        accumulated_bbox = BBox<T>();
        size_t minBin = 1;
        for (size_t i = 0; i < bins->bin_size - 1; i++) {
            Bin<T>& bin = bins->bin[static_cast<unsigned int>(j) * bins->bin_size + i];
            Bin<T>& next_bin = bins->bin[static_cast<unsigned int>(j) * bins->bin_size + i + 1];
            for (int k = 0; k < 3; ++k) {
                accumulated_bbox.bmin[k] = std::min(bin.bbox.bmin[k], accumulated_bbox.bmin[k]);
                accumulated_bbox.bmax[k] = std::max(bin.bbox.bmax[k], accumulated_bbox.bmax[k]);
            }
            count += bin.count;
            // Traversal cost and intersection cost are irrelevant for minimization
            T cost = T(count) * CalculateSurfaceArea(accumulated_bbox.bmin, accumulated_bbox.bmax) + next_bin.cost;
            if (cost < minCost[j]) {
                minCost[j] = cost;
                // Store the beginning of the right partition
                minBin = i + 1;
            }
        }
        cut_pos[j] = T(minBin) * ((bmax[j] - bmin[j]) / T(bins->bin_size)) + bmin[j];
    }
    *minCostAxis = 0;
    if (minCost[0] > minCost[1]) *minCostAxis = 1;
    if (minCost[*minCostAxis] > minCost[2]) *minCostAxis = 2;
    
    return true;
    // FUNCTION_ID: nanort_func014 - END
}

// Explicit template instantiations
template real3<float> vneg<float>(const real3<float>&);
template real3<float> operator*<float>(float, const real3<float>&);
template float vlength<float>(const real3<float>&);
template real3<float> vnormalize<float>(const real3<float>&);
template real3<float> vcross<float>(const real3<float>, const real3<float>);
template float vdot<float>(const real3<float>, const real3<float>);
template real3<float> vsafe_inverse<float>(const real3<float>);
template float CalculateSurfaceArea<float>(const real3<float>&, const real3<float>&);
template void GetBoundingBoxOfTriangle<float>(real3<float>*, real3<float>*, const float*, const unsigned int*, unsigned int);
template const float& safemin<float>(const float&, const float&);
template const float& safemax<float>(const float&, const float&);
template float SAH<float>(size_t, float, size_t, float, float, float, float);
template bool FindCutFromBinBuffer<float>(float*, int*, BinBuffer<float>*, const real3<float>&, const real3<float>&);

} // namespace nanort
