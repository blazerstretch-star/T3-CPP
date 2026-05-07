#ifndef NANORT_FUNCTIONS_H_
#define NANORT_FUNCTIONS_H_

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace nanort {

template <typename T = float>
class real3 {
public:
    real3() {}
    real3(T x) { v[0] = x; v[1] = x; v[2] = x; }
    real3(T xx, T yy, T zz) { v[0] = xx; v[1] = yy; v[2] = zz; }
    explicit real3(const T *p) { v[0] = p[0]; v[1] = p[1]; v[2] = p[2]; }
    
    inline T x() const { return v[0]; }
    inline T y() const { return v[1]; }
    inline T z() const { return v[2]; }
    
    T operator[](int i) const { return v[i]; }
    T &operator[](int i) { return v[i]; }
    
    real3 operator-(const real3 &f2) const {
        return real3(x() - f2.x(), y() - f2.y(), z() - f2.z());
    }
    
    T v[3];
};

// BVH data structures
template <typename T>
class BBox {
public:
    real3<T> bmin;
    real3<T> bmax;
    
    BBox() {
        bmin[0] = bmin[1] = bmin[2] = std::numeric_limits<T>::max();
        bmax[0] = bmax[1] = bmax[2] = -std::numeric_limits<T>::max();
    }
};

template <typename T>
struct Bin {
    BBox<T> bbox;
    size_t count;
    T cost;
    
    Bin() : count(0), cost(0) {}
};

template <typename T>
struct BinBuffer {
    explicit BinBuffer(unsigned int size) {
        bin_size = size;
        bin.resize(3 * size);
        clear();
    }
    
    void clear() {
        std::fill(bin.begin(), bin.end(), Bin<T>());
    }
    
    std::vector<Bin<T>> bin;
    unsigned int bin_size;
    unsigned int pad0;
};

// Vector math functions
template <typename T>
real3<T> vneg(const real3<T> &rhs);

template <typename T>
real3<T> operator*(T f, const real3<T> &v);

template <typename T>
T vlength(const real3<T> &rhs);

template <typename T>
real3<T> vnormalize(const real3<T> &rhs);

template <typename T>
real3<T> vcross(const real3<T> a, const real3<T> b);

template <typename T>
T vdot(const real3<T> a, const real3<T> b);

template <typename T>
real3<T> vsafe_inverse(const real3<T> v);

// Bounding box functions
template <typename T>
T CalculateSurfaceArea(const real3<T> &min, const real3<T> &max);

template <typename T>
void GetBoundingBoxOfTriangle(real3<T> *bmin, real3<T> *bmax,
                              const T *vertices,
                              const unsigned int *faces,
                              unsigned int index);

// Ray-AABB intersection
template <typename T>
bool IntersectRayAABB(T *tminOut, T *tmaxOut,
                     T min_t, T max_t,
                     const T bmin[3], const T bmax[3],
                     real3<T> ray_org, real3<T> ray_inv_dir,
                     int ray_dir_sign[3]);

// NaN-safe comparisons
template <class T>
const T &safemin(const T &a, const T &b);

template <class T>
const T &safemax(const T &a, const T &b);

// SAH calculation
template <typename T>
T SAH(size_t ns1, T leftArea, size_t ns2, T rightArea, T invS, T Taabb, T Ttri);

// BVH construction
template <typename T>
bool FindCutFromBinBuffer(T *cut_pos, int *minCostAxis,
                         BinBuffer<T> *bins,
                         const real3<T> &bmin, const real3<T> &bmax);

} // namespace nanort

#endif // NANORT_FUNCTIONS_H_
