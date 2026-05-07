#pragma once
#include <cmath>
#include <vector>
#include <functional>

namespace NumericalMethods {

// Function pointer type for numerical functions
using FunctionPtr = std::function<double(double)>;

// Root Finding
int sign(double x);
double bisection_root_finder(FunctionPtr f, double a, double b, double precision, unsigned int max_iter);

// Numerical Integration
double midpoint_integration(FunctionPtr f, double a, double b, int N);
double simpson_integration(FunctionPtr f, double a, double b, int N);
double trapezoidal_integration(FunctionPtr f, double a, double b, int N);
double adaptive_midpoint(FunctionPtr f, double a, double b, double precision);
double adaptive_simpson(FunctionPtr f, double a, double b, double precision);
double adaptive_trapezoidal(FunctionPtr f, double a, double b, double precision);

// ODE Solvers
std::vector<double> euler_step(const std::vector<double>& x, double t, double h, 
                                std::function<std::vector<double>(const std::vector<double>&, double)> f);
std::vector<double> runge_kutta_step(const std::vector<double>& x, double t, double h,
                                      std::function<std::vector<double>(const std::vector<double>&, double)> f);

// Random Number Generation
double lcg_rand(unsigned int& seed, unsigned int a, unsigned int c, unsigned int m);
double uniform_random(double min, double max, unsigned int& seed, unsigned int a, unsigned int c, unsigned int m);
double gaussian_box_muller(double mean, double sigma, unsigned int& seed, unsigned int a, unsigned int c, unsigned int m);
double exponential_random(double lambda, unsigned int& seed, unsigned int a, unsigned int c, unsigned int m);

// Monte Carlo Integration
double monte_carlo_mean(FunctionPtr f, double a, double b, int n_points, unsigned int& seed);
double monte_carlo_hit_or_miss(FunctionPtr f, double a, double b, double fmax, int n_points, unsigned int& seed);

}
