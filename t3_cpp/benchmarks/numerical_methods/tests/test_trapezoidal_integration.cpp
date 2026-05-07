#include "../src/numerical_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    using namespace NumericalMethods;
    
    // ========================================
    // PROPERTY 1: Known analytical integrals
    // ========================================
    auto f_sin = [](double x) { return std::sin(x); };
    double result1 = trapezoidal_integration(f_sin, 0, M_PI, 10000);
    assert(std::abs(result1 - 2.0) < 0.0001 && "sin integral should be 2.0");
    
    auto f_x = [](double x) { return x; };
    double result2 = trapezoidal_integration(f_x, 0, 1, 10000);
    assert(std::abs(result2 - 0.5) < 0.00001 && "x integral should be 0.5");
    
    auto f_x2 = [](double x) { return x * x; };
    double result3 = trapezoidal_integration(f_x2, 0, 1, 10000);
    assert(std::abs(result3 - 1.0/3.0) < 0.0001 && "x^2 integral should be 1/3");
    
    // ========================================
    // PROPERTY 2: Constant function (exact for trapezoidal)
    // ========================================
    auto f_const = [](double x) { return 3.0; };
    double result4 = trapezoidal_integration(f_const, 0, 5, 100);
    assert(std::abs(result4 - 15.0) < 1e-10 && "Constant integral should be exact");
    
    // Different constant
    auto f_const2 = [](double x) { return -2.5; };
    double result_const2 = trapezoidal_integration(f_const2, 1, 4, 10);
    assert(std::abs(result_const2 - (-7.5)) < 1e-10 && "Negative constant integral exact");
    
    // ========================================
    // PROPERTY 3: Linear function (exact for trapezoidal)
    // ========================================
    auto f_linear = [](double x) { return 2.0 * x + 3.0; };
    double result_linear = trapezoidal_integration(f_linear, 0, 2, 10);
    // Analytical: integral of (2x+3) from 0 to 2 = [x^2 + 3x] = 4 + 6 = 10
    assert(std::abs(result_linear - 10.0) < 1e-10 && "Linear integral should be exact");
    
    // ========================================
    // PROPERTY 5: Additivity over intervals
    // ========================================
    // integral[a,c] = integral[a,b] + integral[b,c]
    auto f_test = [](double x) { return x * x; };
    double int_0_2 = trapezoidal_integration(f_test, 0, 2, 1000);
    double int_0_1 = trapezoidal_integration(f_test, 0, 1, 500);
    double int_1_2 = trapezoidal_integration(f_test, 1, 2, 500);
    assert(std::abs(int_0_2 - (int_0_1 + int_1_2)) < 0.001 && "Additivity property should hold");
    
    // ========================================
    // PROPERTY 6: Linearity - integral of sum = sum of integrals
    // ========================================
    auto f1 = [](double x) { return x; };
    auto f2 = [](double x) { return x * x; };
    auto f_sum = [](double x) { return x + x * x; };
    
    double int_f1 = trapezoidal_integration(f1, 0, 1, 1000);
    double int_f2 = trapezoidal_integration(f2, 0, 1, 1000);
    double int_sum = trapezoidal_integration(f_sum, 0, 1, 1000);
    assert(std::abs(int_sum - (int_f1 + int_f2)) < 1e-8 && "Linearity property should hold");
    
    // ========================================
    // PROPERTY 7: Scaling property
    // ========================================
    auto f_base = [](double x) { return std::sin(x); };
    auto f_scaled = [](double x) { return 3.0 * std::sin(x); };
    double int_base = trapezoidal_integration(f_base, 0, M_PI, 1000);
    double int_scaled = trapezoidal_integration(f_scaled, 0, M_PI, 1000);
    assert(std::abs(int_scaled - 3.0 * int_base) < 0.001 && "Scaling property should hold");
    
    // ========================================
    // PROPERTY 8: Transcendental functions
    // ========================================
    auto f_exp = [](double x) { return std::exp(x); };
    double result_exp = trapezoidal_integration(f_exp, 0, 1, 10000);
    assert(std::abs(result_exp - (std::exp(1.0) - 1.0)) < 0.0001 && "exp integral accurate");
    
    std::cout << "test_trapezoidal_integration PASSED" << std::endl;
    return 0;
}
