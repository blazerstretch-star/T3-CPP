#include "numerical_functions.h"
#include <cmath>
#include <iostream>
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


namespace NumericalMethods {

// ============================================================================
// ROOT FINDING FUNCTIONS
// ============================================================================

int sign(double x) {
    // FUNCTION_ID: nummethod_func001 - START
    if (x < 0) return -1;
    else return 1;
    // FUNCTION_ID: nummethod_func001 - END
}

double bisection_root_finder(FunctionPtr f, double a, double b, double precision, unsigned int max_iter) {
    // FUNCTION_ID: nummethod_func002 - START
    if (a > b) {
        double temp = a;
        a = b;
        b = temp;
    }
    
    double y_a = f(a);
    double y_b = f(b);
    double c = a + 0.5 * (b - a);
    double y_c = f(c);
    unsigned int i = 0;
    
    if (sign(y_a) * sign(y_b) > 0) {
        std::cerr << "no unique solution in the given interval" << std::endl;
        exit(1);
    }
    
    if (c == 0) return c;
    
    while (std::abs(a - b) >= precision && i < max_iter) {
        if (sign(y_a) * sign(y_c) == -1) {
            b = c;
            y_a = f(a);
            y_b = f(b);
            c = a + 0.5 * (b - a);
            y_c = f(c);
        } else {
            a = c;
            y_a = f(a);
            y_b = f(b);
            c = a + 0.5 * (b - a);
            y_c = f(c);
        }
        i++;
    }
    
    return c;
    // FUNCTION_ID: nummethod_func002 - END
}

// ============================================================================
// NUMERICAL INTEGRATION FUNCTIONS
// ============================================================================

double midpoint_integration(FunctionPtr f, double a, double b, int N) {
    // FUNCTION_ID: nummethod_func003 - START
    double h = (b - a) / N;
    double sum = 0;
    
    for (int i = 0; i < N; ++i) {
        sum += f(a + i * h + (h / 2.));
    }
    return sum * h;
    // FUNCTION_ID: nummethod_func003 - END
}

double simpson_integration(FunctionPtr f, double a, double b, int N) {
    // FUNCTION_ID: nummethod_func004 - START
    if (N % 2 != 0) {
        std::cerr << "N steps must be even for Simpson method" << std::endl;
        exit(1);
    }
    
    double sum = 0;
    double h = (std::abs(b - a)) / N;
    int sign_val = (a > b) ? -1 : 1;
    
    for (int i = 0; i <= N; ++i) {
        if (i == 0 || i == N) {
            sum += f(a + h * i);
        } else if (i % 2 == 0) {
            sum += 2. * f(a + h * i);
        } else {
            sum += 4. * f(a + h * i);
        }
    }
    return sum * h * sign_val * (1. / 3.);
    // FUNCTION_ID: nummethod_func004 - END
}

double trapezoidal_integration(FunctionPtr f, double a, double b, int N) {
    // FUNCTION_ID: nummethod_func005 - START
    double h = (std::fabs(b - a)) / N;
    double sum = 0;
    int sign_val = (a > b) ? -1 : 1;
    
    for (int i = 0; i <= N; ++i) {
        if (i == 0 || i == N) {
            sum += (f(a + h * i)) / 2.;
        } else {
            sum += f(a + h * i);
        }
    }
    
    return sum * h * sign_val;
    // FUNCTION_ID: nummethod_func005 - END
}

double adaptive_midpoint(FunctionPtr f, double a, double b, double precision) {
    // FUNCTION_ID: nummethod_func006 - START
    int N = 1;
    double x = midpoint_integration(f, a, b, N);
    double y = midpoint_integration(f, a, b, 2 * N);
    
    while (((4. / 3.) * std::fabs(x - y)) > precision) {
        N = 2 * N;
        x = midpoint_integration(f, a, b, N);
        y = midpoint_integration(f, a, b, 2 * N);
    }
    return y;
    // FUNCTION_ID: nummethod_func006 - END
}

double adaptive_simpson(FunctionPtr f, double a, double b, double precision) {
    // FUNCTION_ID: nummethod_func007 - START
    int N = 2;
    double x = simpson_integration(f, a, b, N);
    double y = simpson_integration(f, a, b, 2 * N);
    
    while (((16. / 15.) * std::abs(x - y)) > precision) {
        N = 2 * N;
        x = simpson_integration(f, a, b, N);
        y = simpson_integration(f, a, b, 2 * N);
    }
    return y;
    // FUNCTION_ID: nummethod_func007 - END
}

double adaptive_trapezoidal(FunctionPtr f, double a, double b, double precision) {
    // FUNCTION_ID: nummethod_func008 - START
    int N = 1;
    double x = trapezoidal_integration(f, a, b, N);
    double y = trapezoidal_integration(f, a, b, 2 * N);
    
    while (((4. / 3.) * std::abs(x - y)) > precision) {
        N = 2 * N;
        x = trapezoidal_integration(f, a, b, N);
        y = trapezoidal_integration(f, a, b, 2 * N);
    }
    return y;
    // FUNCTION_ID: nummethod_func008 - END
}

// ============================================================================
// ODE SOLVER FUNCTIONS
// ============================================================================

std::vector<double> euler_step(const std::vector<double>& x, double t, double h, std::function<std::vector<double>(const std::vector<double>&, double)> f) {
    // FUNCTION_ID: nummethod_func009 - START
    std::vector<double> fx = f(x, t);
    std::vector<double> result(x.size());
    
    for (size_t i = 0; i < x.size(); ++i) {
        result[i] = x[i] + h * fx[i];
    }
    return result;
    // FUNCTION_ID: nummethod_func009 - END
}

std::vector<double> runge_kutta_step(const std::vector<double>& x, double t, double h, std::function<std::vector<double>(const std::vector<double>&, double)> f) {
    // FUNCTION_ID: nummethod_func010 - START
    std::vector<double> k1 = f(x, t);
    
    std::vector<double> x_k2(x.size());
    for (size_t i = 0; i < x.size(); ++i) {
        x_k2[i] = x[i] + k1[i] * (h / 2.);
    }
    std::vector<double> k2 = f(x_k2, t + h / 2.);
    
    std::vector<double> x_k3(x.size());
    for (size_t i = 0; i < x.size(); ++i) {
        x_k3[i] = x[i] + k2[i] * (h / 2.);
    }
    std::vector<double> k3 = f(x_k3, t + h / 2.);
    
    std::vector<double> x_k4(x.size());
    for (size_t i = 0; i < x.size(); ++i) {
        x_k4[i] = x[i] + k3[i] * h;
    }
    std::vector<double> k4 = f(x_k4, t + h);
    
    std::vector<double> result(x.size());
    for (size_t i = 0; i < x.size(); ++i) {
        result[i] = x[i] + (k1[i] + 2. * k2[i] + 2. * k3[i] + k4[i]) * (h / 6.);
    }
    return result;
    // FUNCTION_ID: nummethod_func010 - END
}

// ============================================================================
// RANDOM NUMBER GENERATION FUNCTIONS
// ============================================================================

double lcg_rand(unsigned int& seed, unsigned int a, unsigned int c, unsigned int m) {
    // FUNCTION_ID: nummethod_func011 - START
    int n = (a * seed + c) % m;
    seed = static_cast<unsigned int>(n);
    return static_cast<double>(n) / m;
    // FUNCTION_ID: nummethod_func011 - END
}

double uniform_random(double min, double max, unsigned int& seed, unsigned int a, unsigned int c, unsigned int m) {
    // FUNCTION_ID: nummethod_func012 - START
    return min + (max - min) * lcg_rand(seed, a, c, m);
    // FUNCTION_ID: nummethod_func012 - END
}

double gaussian_box_muller(double mean, double sigma, unsigned int& seed, unsigned int a, unsigned int c, unsigned int m) {
    // FUNCTION_ID: nummethod_func013 - START
    double s = lcg_rand(seed, a, c, m);
    double t = lcg_rand(seed, a, c, m);
    double x = std::sqrt(-2 * std::log(s)) * std::cos(2. * M_PI * t);
    return mean + x * sigma;
    // FUNCTION_ID: nummethod_func013 - END
}

double exponential_random(double lambda, unsigned int& seed, unsigned int a, unsigned int c, unsigned int m) {
    // FUNCTION_ID: nummethod_func014 - START
    return -1. / lambda * std::log(1 - lcg_rand(seed, a, c, m));
    // FUNCTION_ID: nummethod_func014 - END
}

// ============================================================================
// MONTE CARLO INTEGRATION FUNCTIONS
// ============================================================================

double monte_carlo_mean(FunctionPtr f, double a, double b, int n_points, unsigned int& seed) {
    // FUNCTION_ID: nummethod_func015 - START
    unsigned int lcg_a = 1664525;
    unsigned int lcg_c = 1013904223;
    unsigned int lcg_m = 2147483648;
    
    double sum = 0.;
    for (int i = 0; i < n_points; i++) {
        double x = uniform_random(a, b, seed, lcg_a, lcg_c, lcg_m);
        sum += f(x);
    }
    return (std::fabs(b - a) * sum) / n_points;
    // FUNCTION_ID: nummethod_func015 - END
}

double monte_carlo_hit_or_miss(FunctionPtr f, double a, double b, double fmax, int n_points, unsigned int& seed) {
    // FUNCTION_ID: nummethod_func016 - START
    unsigned int lcg_a = 1664525;
    unsigned int lcg_c = 1013904223;
    unsigned int lcg_m = 2147483648;
    
    int n_hit = 0;
    int n_tot = 0;
    
    for (int i = 0; i < n_points; i++) {
        n_tot++;
        double y = uniform_random(0, fmax, seed, lcg_a, lcg_c, lcg_m);
        double x = uniform_random(a, b, seed, lcg_a, lcg_c, lcg_m);
        double f_x = f(x);
        if (f_x > y) n_hit++;
    }
    return std::fabs(b - a) * fmax * (static_cast<double>(n_hit) / static_cast<double>(n_tot));
    // FUNCTION_ID: nummethod_func016 - END
}

}
