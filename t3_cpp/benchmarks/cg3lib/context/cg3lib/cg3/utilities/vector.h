/*
 * This file is part of cg3lib: https://github.com/cg3hci/cg3lib
 * This Source Code Form is subject to the terms of the GNU GPL 3.0
 *
 * @author Alessandro Muntoni (muntoni.alessandro@gmail.com)
 */

#ifndef CG3_VECTOR_H
#define CG3_VECTOR_H

#include <vector>

namespace cg3 {

template <typename T, typename ...A>
int binarySearch(const T &n, const std::vector<T, A...> &v);

template <typename T, typename ...A>
std::vector<size_t> sortIndexes(const std::vector<T, A...> &v);

template <typename T, typename ...A>
bool contains(const std::vector<T, A...>& v, const T& value);

template <typename T, typename ...A>
int positionOfElement(const std::vector<T, A...>& v, const T& value);


} //namespace cg3

#include "vector.inl"

#endif // CG3_VECTOR_H

template <typename T>
inline void normalize(std::vector<T>& vec) {
    // FUNCTION_ID: cg3lib_func017 - START
    T sum = std::accumulate(vec.begin(), vec.end(), T(0));
    if (sum != T(0)) for (auto& v : vec) v /= sum;
    // FUNCTION_ID: cg3lib_func017 - END
}

template <typename T>
inline double length(const std::vector<T>& vec) {
    // FUNCTION_ID: cg3lib_func018 - START
    return std::sqrt(std::inner_product(vec.begin(), vec.end(), vec.begin(), 0.0));
    // FUNCTION_ID: cg3lib_func018 - END
}
