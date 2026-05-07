// Standalone functions extracted from C++QED repository
// Copyright András Vukics 2006–2023. Distributed under the Boost Software License, Version 1.0.

#include "cppqed_functions.h"
#include <boost/math/special_functions/hermite.hpp>
#include <boost/math/special_functions/laguerre.hpp>
#include <boost/math/special_functions/factorials.hpp>
#include <boost/math/special_functions/binomial.hpp>
#include <stdexcept>
#include <numbers>
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


using namespace boost::math;

namespace {

constexpr double infinity = std::numeric_limits<double>::infinity();

}

// Function 1: Calculate Wigner function weight w(n, r, k)
// Original: quantumdata::details::w in DistributionFunctions.cc (line 52-56)
double calculate_wigner_weight(size_t n, double r, size_t k) {
  // FUNCTION_ID: cppqed_func001 - START
  const double sqrR = sqr(r);
  return minusOneToThePowerOf(n) / PI * std::sqrt(factorial<double>(n) / factorial<double>(n + k)) * 
         std::exp(-2 * sqrR) * std::pow(2 * r, k) * laguerre(n, k, 4 * sqrR);
  // FUNCTION_ID: cppqed_func001 - END
}

// Function 2: Fill array with Hermite polynomials using recurrence
// Original: fillWithHermite in DistributionFunctions.cc (line 35-41)
std::vector<double> fill_hermite_array(size_t dim, double x) {
  // FUNCTION_ID: cppqed_func002 - START
  std::vector<double> res(2 * dim - 1);
  res[0] = hermite(0, x);
  res[1] = hermite(1, x);
  for (unsigned l = 1; l < res.size() - 1; ++l)
    res[l + 1] = hermite_next(l, x, res[l], res[l - 1]);
  return res;
  // FUNCTION_ID: cppqed_func002 - END
}

// Function 3: Calculate Wigner function kernel element
// Original: WignerFunctionKernelOld::operator() in DistributionFunctions.cc (line 63-75)
dcomp calculate_wigner_kernel(size_t m, size_t n, const std::vector<double>& hermite_m2x, const std::vector<double>& hermite_2y) {
  // FUNCTION_ID: cppqed_func003 - START
  dcomp res(0);
  for (size_t u = 0; u <= m; ++u)
    for (size_t v = 0; v <= n; ++v)
      res += binomial_coefficient<double>(m, u) *
             binomial_coefficient<double>(n, v) *
             minusOneToThePowerOf(v) *
             std::pow(1i, u + v) *
             hermite_m2x[u + v] *
             hermite_2y[n + m - u - v];
  return res;
  // FUNCTION_ID: cppqed_func003 - END
}

// Function 4: Apply sparse matrix to state vector
// Original: quantumoperator::SparseMatrix::operator() in SparseMatrix.cc (line 5-12)
void apply_sparse_matrix(std::span<const dcomp> psi, std::span<dcomp> dpsidt, const std::list<std::tuple<size_t, size_t, dcomp>>& elements) {
  // FUNCTION_ID: cppqed_func004 - START
  for (auto&& [j, i, value] : elements) {
#ifndef NDEBUG
    if (size_t s = std::size(psi); j >= s || i >= s)
      throw std::range_error("SparseMatrix::operator() index values:" + std::to_string(j) + ", " + std::to_string(i) + "; extent: " + std::to_string(s));
#endif
    dpsidt[j] += value * psi[i];
  }
  // FUNCTION_ID: cppqed_func004 - END
}

// Function 5: Calculate particular solution for driven damped harmonic oscillator
// Original: cppqedutils::ddho::_::c in DrivenDampedHarmonicOscillator.cc (line 8-11)
dcomp calculate_ddho_particular_solution(double t, double gamma, double omega) {
  // FUNCTION_ID: cppqed_func005 - START
  return std::exp(1i * omega * t) / (1.0 + 2.0 * 1i * gamma * omega - sqr(omega));
  // FUNCTION_ID: cppqed_func005 - END
}

// Function 6: Calculate diagonal elements for spin raising operator s+
// Original: spin::splus in Spin.cc (line 13-22)
std::vector<double> calculate_spin_plus_diagonal(size_t twoS, size_t dim) {
  // FUNCTION_ID: cppqed_func006 - START
  if (dim == 0 || dim > twoS + 1)
    dim = twoS + 1;
  std::vector<double> res(dim - 1);
  // i=m+s (m is the magnetic quantum number)
  for (size_t i = 0; i < res.size(); ++i)
    res[i] = std::sqrt((twoS - i) * (i + 1.0));
  return res;
  // FUNCTION_ID: cppqed_func006 - END
}

// Function 7: Calculate mode annihilation operator diagonal
// Original: mode::aOp in Mode.cc (line 12-20)
std::vector<dcomp> calculate_mode_annihilation_diagonal(size_t cutoff) {
  // FUNCTION_ID: cppqed_func007 - START
  std::vector<dcomp> res(cutoff - 1);
  for (size_t i = 0; i < res.size(); ++i)
    res[i] = std::sqrt(double(i + 1));
  return res;
  // FUNCTION_ID: cppqed_func007 - END
}

// Function 8: Calculate coherent state element
// Original: coherentElement in Mode.cc (line 99-103)
dcomp calculate_coherent_element(unsigned long n, dcomp alpha) {
  // FUNCTION_ID: cppqed_func008 - START
  if (n == 0)
    return 1.0;
  if (n < max_factorial<double>::value)
    return std::pow(alpha, n) / std::sqrt(factorial<double>(n));
  return std::pow(2 * n * std::numbers::pi, -0.25) * std::pow(alpha / std::sqrt(n / std::numbers::e), n);
  // FUNCTION_ID: cppqed_func008 - END
}

// Function 9: Calculate mode function value
// Original: modeFunction in ModeFunction.cc (line 36-43)
dcomp calculate_mode_function(int mf_type, double x) {
  // FUNCTION_ID: cppqed_func009 - START
  switch (mf_type) {
  case 0: // MFT_SIN
    return std::sin(x);
  case 1: // MFT_COS
    return std::cos(x);
  case 2: // MFT_PLUS
    return std::exp(1i * x);
  case 3: // MFT_MINUS
    return std::exp(-1i * x);
  }
  return std::exp(-1i * x);
  // FUNCTION_ID: cppqed_func009 - END
}

// Function 10: Apply photon loss jump operator
// Original: mode::aJump in Mode.cc (line 37-41)
std::vector<dcomp> apply_photon_loss_jump(const std::vector<dcomp>& psi, double kappa, double nTh) {
  // FUNCTION_ID: cppqed_func010 - START
  double fact = std::sqrt(2.0 * kappa * (nTh + 1));
  std::vector<dcomp> result(psi.size());
  for (size_t n = 0; n < psi.size() - 1; ++n)
    result[n] = fact * std::sqrt(n + 1) * psi[n + 1];
  result[psi.size() - 1] = 0;
  return result;
  // FUNCTION_ID: cppqed_func010 - END
}
