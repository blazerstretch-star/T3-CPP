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
    double result1 = adaptive_trapezoidal(f_sin, 0, M_PI, 0.0001);
    assert(std::abs(result1 - 2.0) < 0.001 && "sin integral should meet precision");
    
    auto f_x = [](double x) { return x; };
    double result2 = adaptive_trapezoidal(f_x, 0, 1, 0.0001);
    assert(std::abs(result2 - 0.5) < 0.001 && "x integral should meet precision");
    
    auto f_x2 = [](double x) { return x * x; };
    double result3 = adaptive_trapezoidal(f_x2, 0, 1, 0.0001);
    assert(std::abs(result3 - 1.0/3.0) < 0.001 && "x^2 integral should meet precision");
    
    // ========================================
    // PROPERTY 2: Linear functions are exact
    // ========================================
    auto f_linear = [](double x) { return 2.0 * x + 3.0; };
    double result_linear = adaptive_trapezoidal(f_linear, 0, 2, 0.01);
    // integral of (2x+3) from 0 to 2 = [x^2 + 3x] = 4 + 6 = 10
    assert(std::abs(result_linear - 10.0) < 1e-8 && "Linear integral should be exact");
    
    // ========================================
    // PROPERTY 3: Constant functions are exact
    // ========================================
    auto f_const = [](double x) { return 5.0; };
    double result_const = adaptive_trapezoidal(f_const, 0, 3, 0.01);
    assert(std::abs(result_const - 15.0) < 1e-8 && "Constant integral should be exact");
    
    // ========================================
    // PROPERTY 4: Tighter precision gives more accurate result
    // ========================================
    double result_loose = adaptive_trapezoidal(f_sin, 0, M_PI, 0.01);
    double result_tight = adaptive_trapezoidal(f_sin, 0, M_PI, 0.0001);
    
    double error_loose = std::abs(result_loose - 2.0);
    double error_tight = std::abs(result_tight - 2.0);
    
    // Tighter precision should give better or equal accuracy
    assert(error_tight <= error_loose + 0.001 && "Tighter precision should be more accurate");
    
    // ========================================
    // PROPERTY 5: Additivity over intervals
    // ========================================
    auto f_test = [](double x) { return x * x; };
    double int_0_2 = adaptive_trapezoidal(f_test, 0, 2, 0.0001);
    double int_0_1 = adaptive_trapezoidal(f_test, 0, 1, 0.0001);
    double int_1_2 = adaptive_trapezoidal(f_test, 1, 2, 0.0001);
    assert(std::abs(int_0_2 - (int_0_1 + int_1_2)) < 0.001 && "Additivity should hold");
    
    // ========================================
    // PROPERTY 6: Polynomial accuracy
    // ========================================
    auto f_x3 = [](double x) { return x * x * x; };
    double result_x3 = adaptive_trapezoidal(f_x3, 0, 1, 0.0001);
    assert(std::abs(result_x3 - 0.25) < 0.001 && "x^3 integral should be 0.25");
    
    // ========================================
    // PROPERTY 7: Transcendental functions
    // ========================================
    auto f_exp = [](double x) { return std::exp(x); };
    double result_exp = adaptive_trapezoidal(f_exp, 0, 1, 0.0001);
    assert(std::abs(result_exp - (std::exp(1.0) - 1.0)) < 0.001 && "exp integral accurate");
    
    auto f_cos = [](double x) { return std::cos(x); };
    double result_cos = adaptive_trapezoidal(f_cos, 0, M_PI/2, 0.0001);
    assert(std::abs(result_cos - 1.0) < 0.001 && "cos integral should be 1.0");
    
    // ========================================
    // PROPERTY 8: Different intervals
    // ========================================
    double result_diff = adaptive_trapezoidal(f_x, 2, 5, 0.0001);
    // integral of x from 2 to 5 = [x^2/2] = 25/2 - 4/2 = 10.5
    assert(std::abs(result_diff - 10.5) < 1e-8 && "Different interval should work");
    
    // ========================================
    // PROPERTY 9: Scaling property
    // ========================================
    auto f_base = [](double x) { return std::sin(x); };
    auto f_scaled = [](double x) { return 2.5 * std::sin(x); };
    double int_base = adaptive_trapezoidal(f_base, 0, M_PI, 0.0001);
    double int_scaled = adaptive_trapezoidal(f_scaled, 0, M_PI, 0.0001);
    assert(std::abs(int_scaled - 2.5 * int_base) < 0.001 && "Scaling property should hold");
    
    // ========================================
    // PROPERTY 10: Linearity
    // ========================================
    auto f1 = [](double x) { return x; };
    auto f2 = [](double x) { return x * x; };
    auto f_sum = [](double x) { return x + x * x; };
    
    double int_f1 = adaptive_trapezoidal(f1, 0, 1, 0.0001);
    double int_f2 = adaptive_trapezoidal(f2, 0, 1, 0.0001);
    double int_sum = adaptive_trapezoidal(f_sum, 0, 1, 0.0001);
    assert(std::abs(int_sum - (int_f1 + int_f2)) < 0.001 && "Linearity property should hold");
    
    std::cout << "test_adaptive_trapezoidal PASSED" << std::endl;
    return 0;
}
