#include "../src/numerical_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    using namespace NumericalMethods;
    
    // ========================================
    // PROPERTY 1: Integral of sin(x) from 0 to pi = 2
    // ========================================
    auto f_sin = [](double x) { return std::sin(x); };
    double result1 = simpson_integration(f_sin, 0, M_PI, 1000);
    assert(std::abs(result1 - 2.0) < 0.0001 && "sin integral should be 2.0");
    
    // ========================================
    // PROPERTY 2: Polynomial integrals (exact for degree <= 3)
    // ========================================
    // Simpson's rule is exact for polynomials up to degree 3
    auto f_x2 = [](double x) { return x * x; };
    double result2 = simpson_integration(f_x2, 0, 1, 100);
    assert(std::abs(result2 - 1.0/3.0) < 1e-10 && "x^2 integral should be exact");
    
    auto f_x3 = [](double x) { return x * x * x; };
    double result3 = simpson_integration(f_x3, 0, 1, 100);
    assert(std::abs(result3 - 0.25) < 1e-10 && "x^3 integral should be exact");
    
    // Linear function (exact)
    auto f_x = [](double x) { return x; };
    double result_linear = simpson_integration(f_x, 0, 1, 100);
    assert(std::abs(result_linear - 0.5) < 1e-10 && "Linear integral should be exact");
    
    // ========================================
    // PROPERTY 3: Constant function (exact)
    // ========================================
    auto f_const = [](double x) { return 7.0; };
    double result_const = simpson_integration(f_const, 2, 5, 100);
    assert(std::abs(result_const - 21.0) < 1e-10 && "Constant integral should be exact");
    
    // ========================================
    // PROPERTY 5: Linearity - integral of sum = sum of integrals
    // ========================================
    auto f1 = [](double x) { return x; };
    auto f2 = [](double x) { return x * x; };
    auto f_sum = [](double x) { return x + x * x; };
    
    double int_f1 = simpson_integration(f1, 0, 1, 100);
    double int_f2 = simpson_integration(f2, 0, 1, 100);
    double int_sum = simpson_integration(f_sum, 0, 1, 100);
    assert(std::abs(int_sum - (int_f1 + int_f2)) < 1e-9 && "Linearity property should hold");
    
    // ========================================
    // PROPERTY 6: Transcendental functions with known values
    // ========================================
    auto f_exp = [](double x) { return std::exp(x); };
    double result4 = simpson_integration(f_exp, 0, 1, 1000);
    assert(std::abs(result4 - (std::exp(1.0) - 1.0)) < 0.00001 && "exp integral accurate");
    
    auto f_cos = [](double x) { return std::cos(x); };
    double result_cos = simpson_integration(f_cos, 0, M_PI/2, 1000);
    assert(std::abs(result_cos - 1.0) < 0.00001 && "cos integral should be 1.0");
    
    // ========================================
    // PROPERTY 7: Zero integral for symmetric odd functions
    // ========================================
    auto f_odd = [](double x) { return x * x * x; };
    double result_symmetric = simpson_integration(f_odd, -1, 1, 100);
    assert(std::abs(result_symmetric) < 1e-10 && "Odd function symmetric integral should be zero");
    
    std::cout << "test_simpson_integration PASSED" << std::endl;
    return 0;
}
