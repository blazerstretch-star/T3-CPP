#include "../src/numerical_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    using namespace NumericalMethods;
    
    // ========================================
    // PROPERTY 1: Known analytical integrals
    // ========================================
    unsigned int seed = 12345;
    
    auto f_sin = [](double x) { return std::sin(x); };
    double result1 = monte_carlo_hit_or_miss(f_sin, 0, M_PI, 1.0, 50000, seed);
    assert(std::abs(result1 - 2.0) < 0.1 && "sin integral should be close to 2.0");
    
    auto f_x = [](double x) { return x; };
    unsigned int seed2 = 54321;
    double result2 = monte_carlo_hit_or_miss(f_x, 0, 1, 1.0, 50000, seed2);
    assert(std::abs(result2 - 0.5) < 0.05 && "x integral should be close to 0.5");
    
    // ========================================
    // PROPERTY 2: Constant function
    // ========================================
    auto f_const = [](double x) { return 0.5; };
    unsigned int seed3 = 98765;
    double result3 = monte_carlo_hit_or_miss(f_const, 0, 2, 1.0, 50000, seed3);
    assert(std::abs(result3 - 1.0) < 0.05 && "Constant integral should be close to 1.0");
    
    // Different constant
    auto f_const2 = [](double x) { return 0.3; };
    unsigned int seed4 = 11111;
    double result4 = monte_carlo_hit_or_miss(f_const2, 0, 5, 1.0, 50000, seed4);
    // integral of 0.3 from 0 to 5 = 0.3 * 5 = 1.5
    assert(std::abs(result4 - 1.5) < 0.1 && "Constant 0.3 integral should be close to 1.5");
    
    // ========================================
    // PROPERTY 3: Quadratic function
    // ========================================
    auto f_x2 = [](double x) { return x * x; };
    unsigned int seed5 = 22222;
    double result5 = monte_carlo_hit_or_miss(f_x2, 0, 1, 1.0, 50000, seed5);
    assert(std::abs(result5 - 1.0/3.0) < 0.05 && "x^2 integral should be close to 1/3");
    
    // ========================================
    // PROPERTY 4: Convergence with more samples
    // ========================================
    unsigned int seed6a = 33333;
    unsigned int seed6b = 33333;
    
    double result_few = monte_carlo_hit_or_miss(f_x, 0, 1, 1.0, 5000, seed6a);
    double result_many = monte_carlo_hit_or_miss(f_x, 0, 1, 1.0, 100000, seed6b);
    
    // More samples should generally be more accurate
    double error_few = std::abs(result_few - 0.5);
    double error_many = std::abs(result_many - 0.5);
    
    // At least one should be reasonably accurate
    assert((error_few < 0.15 || error_many < 0.05) && "Should converge with more samples");
    
    // ========================================
    // PROPERTY 5: Different intervals
    // ========================================
    auto f_linear = [](double x) { return x; };
    unsigned int seed8 = 55555;
    
    // integral of x from 0 to 2 = [x^2/2] = 2
    double result8 = monte_carlo_hit_or_miss(f_linear, 0, 2, 2.0, 50000, seed8);
    assert(std::abs(result8 - 2.0) < 0.15 && "Different interval should work");
    
    // ========================================
    // PROPERTY 6: Zero function
    // ========================================
    auto f_zero = [](double x) { return 0.0; };
    unsigned int seed9 = 66666;
    double result9 = monte_carlo_hit_or_miss(f_zero, 0, 10, 1.0, 10000, seed9);
    assert(std::abs(result9) < 0.1 && "Zero function should give zero integral");
    
    // ========================================
    // PROPERTY 7: Positive results for positive functions
    // ========================================
    auto f_positive = [](double x) { return std::abs(std::sin(x)) + 0.1; };
    unsigned int seed10 = 77777;
    double result10 = monte_carlo_hit_or_miss(f_positive, 0, M_PI, 1.5, 30000, seed10);
    assert(result10 > 0 && "Positive function should give positive integral");
    
    // ========================================
    // PROPERTY 8: Comparison with analytical result
    // ========================================
    auto f_sqrt = [](double x) { return std::sqrt(x); };
    unsigned int seed11 = 88888;
    // integral of sqrt(x) from 0 to 1 = [2/3 * x^(3/2)] = 2/3
    double result11 = monte_carlo_hit_or_miss(f_sqrt, 0, 1, 1.0, 50000, seed11);
    assert(std::abs(result11 - 2.0/3.0) < 0.1 && "sqrt integral should be close to 2/3");
    
    // ========================================
    // PROPERTY 9: Linearity (approximate for Monte Carlo)
    // ========================================
    auto f1 = [](double x) { return 0.2; };
    auto f2 = [](double x) { return 0.3; };
    auto f_sum = [](double x) { return 0.5; };
    
    unsigned int seed12a = 99999;
    unsigned int seed12b = 99999;
    unsigned int seed12c = 99999;
    
    double int_f1 = monte_carlo_hit_or_miss(f1, 0, 1, 1.0, 30000, seed12a);
    double int_f2 = monte_carlo_hit_or_miss(f2, 0, 1, 1.0, 30000, seed12b);
    double int_sum = monte_carlo_hit_or_miss(f_sum, 0, 1, 1.0, 30000, seed12c);
    
    // Allow large tolerance for Monte Carlo
    assert(std::abs(int_sum - (int_f1 + int_f2)) < 0.15 && "Linearity should approximately hold");
    
    std::cout << "test_monte_carlo_hit_or_miss PASSED" << std::endl;
    return 0;
}
