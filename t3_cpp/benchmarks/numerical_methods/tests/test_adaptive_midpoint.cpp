#include "../src/numerical_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    using namespace NumericalMethods;
    
    // ========================================
    // PROPERTY 1: Achieves target precision
    // ========================================
    auto f_sin = [](double x) { return std::sin(x); };
    double result1 = adaptive_midpoint(f_sin, 0, M_PI, 0.0001);
    assert(std::abs(result1 - 2.0) < 0.001 && "sin integral should meet precision");
    
    auto f_x2 = [](double x) { return x * x; };
    double result2 = adaptive_midpoint(f_x2, 0, 1, 0.0001);
    assert(std::abs(result2 - 1.0/3.0) < 0.001 && "x^2 integral should meet precision");
    
    auto f_exp = [](double x) { return std::exp(x); };
    double result3 = adaptive_midpoint(f_exp, 0, 1, 0.0001);
    assert(std::abs(result3 - (std::exp(1.0) - 1.0)) < 0.001 && "exp integral should meet precision");
    
    // ========================================
    // PROPERTY 2: Tighter precision gives more accurate result
    // ========================================
    double result_loose = adaptive_midpoint(f_sin, 0, M_PI, 0.01);
    double result_tight = adaptive_midpoint(f_sin, 0, M_PI, 0.0001);
    
    double error_loose = std::abs(result_loose - 2.0);
    double error_tight = std::abs(result_tight - 2.0);
    
    // Tighter precision should give better or equal accuracy
    assert(error_tight <= error_loose + 0.001 && "Tighter precision should be more accurate");
    
    // ========================================
    // PROPERTY 3: Constant function (should be exact)
    // ========================================
    auto f_const = [](double x) { return 7.0; };
    double result_const = adaptive_midpoint(f_const, 0, 5, 0.01);
    assert(std::abs(result_const - 35.0) < 0.001 && "Constant integral should be exact");
    
    // ========================================
    // PROPERTY 4: Linear function (should be very accurate)
    // ========================================
    auto f_linear = [](double x) { return 2.0 * x + 3.0; };
    double result_linear = adaptive_midpoint(f_linear, 0, 2, 0.0001);
    // integral of (2x+3) from 0 to 2 = [x^2 + 3x] = 4 + 6 = 10
    assert(std::abs(result_linear - 10.0) < 0.001 && "Linear integral should be accurate");
    
    // ========================================
    // PROPERTY 5: Additivity over intervals
    // ========================================
    auto f_test = [](double x) { return std::sin(x); };
    double int_0_pi = adaptive_midpoint(f_test, 0, M_PI, 0.0001);
    double int_0_half = adaptive_midpoint(f_test, 0, M_PI/2, 0.0001);
    double int_half_pi = adaptive_midpoint(f_test, M_PI/2, M_PI, 0.0001);
    assert(std::abs(int_0_pi - (int_0_half + int_half_pi)) < 0.01 && "Additivity should hold");
    
    // ========================================
    // PROPERTY 6: Polynomial accuracy
    // ========================================
    auto f_x3 = [](double x) { return x * x * x; };
    double result_x3 = adaptive_midpoint(f_x3, 0, 1, 0.0001);
    assert(std::abs(result_x3 - 0.25) < 0.001 && "x^3 integral should be 0.25");
    
    // ========================================
    // PROPERTY 7: Transcendental functions
    // ========================================
    auto f_cos = [](double x) { return std::cos(x); };
    double result_cos = adaptive_midpoint(f_cos, 0, M_PI/2, 0.0001);
    assert(std::abs(result_cos - 1.0) < 0.001 && "cos integral should be 1.0");
    
    // ========================================
    // PROPERTY 8: Different intervals
    // ========================================
    auto f_x = [](double x) { return x; };
    double result_diff = adaptive_midpoint(f_x, 2, 5, 0.0001);
    // integral of x from 2 to 5 = [x^2/2] = 25/2 - 4/2 = 10.5
    assert(std::abs(result_diff - 10.5) < 0.001 && "Different interval should work");
    
    // ========================================
    // PROPERTY 9: Scaling property
    // ========================================
    auto f_base = [](double x) { return x * x; };
    auto f_scaled = [](double x) { return 3.0 * x * x; };
    double int_base = adaptive_midpoint(f_base, 0, 1, 0.0001);
    double int_scaled = adaptive_midpoint(f_scaled, 0, 1, 0.0001);
    assert(std::abs(int_scaled - 3.0 * int_base) < 0.001 && "Scaling property should hold");
    
    std::cout << "test_adaptive_midpoint PASSED" << std::endl;
    return 0;
}
