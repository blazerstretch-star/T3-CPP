#ifndef myalgebra
#define myalgebra

#include <iostream>
#include <vector>
#include <cassert>
#include <algorithm>  

// MANUAL IMPLEMENTATION OF LINEAR ALGEBRA OPERATIONS

// overloading operator +
template <typename T> std::vector<T> operator+(const std::vector<T> &a, const std::vector<T> &b) {
  
    assert(a.size() == b.size()); 
    std::vector<T> result(a.size()); 
  
    for (int i = 0; i < static_cast<int>(a.size()); i++) result[i] = a[i] + b[i];
  
  return result;
}

// element wise difference 
template <typename T> std::vector<T> operator-(const std::vector<T> &a, const std::vector<T> &b) {
  
  assert(a.size() == b.size());
  std::vector<T> result(a.size());
  
  for (int i = 0; i < static_cast<int>(a.size()); i++) result[i] = a[i] - b[i];
  return result;
}

// sclar product
template <typename T> T operator*(const std::vector<T> &a, const std::vector<T> &b) {
  
  assert(a.size() == b.size());
  T sum = 0 ;
  for (int i = 0; i < static_cast<int>(a.size()); i++) sum += a[i] * b[i];
  return sum;
}

// product scalar * vector
template <typename T> std::vector<T> operator*( T c , const std::vector<T> &a) {
  
  std::vector<T> result(a.size());
  
  for (int i = 0; i < static_cast<int>(a.size()); i++) result[i] = c * a[i];
  return result;
}

template <typename T> std::vector<T> operator*( const std::vector<T> &a , T c) {
  
  std::vector<T> result(a.size());
  for (int i = 0; i < static_cast<int>(a.size()); i++) result[i] = c * a[i];
  
  return result;
}
// division scalar - vector
template <typename T> std::vector<T> operator/( const std::vector<T> &a , T c) {
  
  std::vector<T> result(a.size());
  for (int i = 0; i < static_cast<int>(a.size()); i++) result[i] = a[i] / c ;

  return result;
}


template <typename T> std::vector<T>& operator+=(std::vector<T>& a, const std::vector<T>& b) {
  
  assert(a.size() == b.size());
  
  for (int i = 0; i < static_cast<int>(a.size()); i++) a[i] += b[i];
  
  return a;
}


template <typename T> std::vector<T>& operator-=(std::vector<T>& a, const std::vector<T>& b) {
  
  assert(a.size() == b.size());
  
  for (int i = 0; i < static_cast<int>(a.size()); i++) a[i] -= b[i];
  return a;
}

namespace myVectUtils {
    
    template <typename T> void Print( const std::vector<T> &v ) {
    
    std::cout << "Printing vector" << std::endl;
    for ( auto it = v.begin() ; it != v.end() ; it++ ) std::cout << *it << " " ;
    std::cout << std::endl;
    std::cout << "End of printing vector" << std::endl;
    
  };
}


#endif
