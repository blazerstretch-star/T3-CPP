// Standalone functions extracted from C++QED repository
// Copyright András Vukics 2006–2023. Distributed under the Boost Software License, Version 1.0.

#ifndef CPPQED_FUNCTIONS_H
#define CPPQED_FUNCTIONS_H

#include <complex>
#include <vector>
#include <cmath>
#include <span>
#include <tuple>
#include <list>
#include <array>

using dcomp = std::complex<double>;
using namespace std::complex_literals;
constexpr double PI = 3.14159265358979323846;

// Helper functions
template<typename T>
inline T sqr(T x) { return x * x; }

template<typename T>
inline T sqrAbs(std::complex<T> z) { return std::norm(z); }

inline int minusOneToThePowerOf(size_t n) { return (n % 2) ? -1 : 1; }

// Function declarations
double calculate_wigner_weight(size_t n, double r, size_t k);
std::vector<double> fill_hermite_array(size_t dim, double x);
dcomp calculate_wigner_kernel(size_t m, size_t n, const std::vector<double>& hermite_m2x, const std::vector<double>& hermite_2y);
void apply_sparse_matrix(std::span<const dcomp> psi, std::span<dcomp> dpsidt, const std::list<std::tuple<size_t, size_t, dcomp>>& elements);
dcomp calculate_ddho_particular_solution(double t, double gamma, double omega);
std::vector<double> calculate_spin_plus_diagonal(size_t twoS, size_t dim);
std::vector<dcomp> calculate_mode_annihilation_diagonal(size_t cutoff);
dcomp calculate_coherent_element(unsigned long n, dcomp alpha);
dcomp calculate_mode_function(int mf_type, double x);
std::vector<dcomp> apply_photon_loss_jump(const std::vector<dcomp>& psi, double kappa, double nTh);

#endif // CPPQED_FUNCTIONS_H
