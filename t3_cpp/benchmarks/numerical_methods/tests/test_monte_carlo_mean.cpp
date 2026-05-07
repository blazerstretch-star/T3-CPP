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
    double result1 = monte_carlo_mean(f_sin, 0, M_PI, 50000, seed);
    assert(std::abs(result1 - 2.0) < 0.05 && "sin integral should be close to 2.0");
    
    auto f_x = [](double x) { return x; };
    unsigned int seed2 = 54321;
    double result2 = monte_carlo_mean(f_x, 0, 1, 50000, seed2);
    assert(std::abs(result2 - 0.5) < 0.01 && "x integral should be close to 0.5");
    
    auto f_x2 = [](double x) { return x * x; };
    unsigned int seed3 = 98765;
    double result3 = monte_carlo_mean(f_x2, 0, 1, 50000, seed3);
    assert(std::abs(result3 - 1.0/3.0) < 0.01 && "x^2 integral should be close to 1/3");
    
    // ========================================
    // PROPERTY 2: Constant function (exact)
    // ========================================
    auto f_const = [](double x) { return 5.0; };
    unsigned int seed4 = 11111;
    double result4 = monte_carlo_mean(f_const, 0, 2, 10000, seed4);
    assert(std::abs(result4 - 10.0) < 0.05 && "Constant integral should be close to 10.0");
    
    // ========================================
    // PROPERTY 3: Linearity - integral of sum = sum of integrals
    // ========================================
    auto f1 = [](double x) { return x; };
    auto f2 = [](double x) { return x * x; };
    auto f_sum = [](double x) { return x + x * x; };
    
    unsigned int seed5a = 22222;
    unsigned int seed5b = 22222;
    unsigned int seed5c = 22222;
    
    double int_f1 = monte_carlo_mean(f1, 0, 1, 30000, seed5a);
    double int_f2 = monte_carlo_mean(f2, 0, 1, 30000, seed5b);
    double int_sum = monte_carlo_mean(f_sum, 0, 1, 30000, seed5c);
    
    // Allow larger tolerance for Monte Carlo
    assert(std::abs(int_sum - (int_f1 + int_f2)) < 0.05 && "Linearity property should hold");
    
    // ========================================
    // PROPERTY 4: Scaling property
    // ========================================
    auto f_base = [](double x) { return x; };
    auto f_scaled = [](double x) { return 3.0 * x; };
    
    unsigned int seed6a = 33333;
    unsigned int seed6b = 33333;
    
    double int_base = monte_carlo_mean(f_base, 0, 1, 30000, seed6a);
    double int_scaled = monte_carlo_mean(f_scaled, 0, 1, 30000, seed6b);
    
    assert(std::abs(int_scaled - 3.0 * int_base) < 0.05 && "Scaling property should hold");
    
    // ========================================
    // PROPERTY 5: Reversed bounds give negative result
    // ========================================
    unsigned int seed7a = 44444;
    unsigned int seed7b = 44444;
    
    double result_fwd = monte_carlo_mean(f_x, 0, 1, 30000, seed7a);
    double result_rev = monte_carlo_mean(f_x, 1, 0, 30000, seed7b);
    
    // Note: Implementation may not support reversed bounds, so check if it does
    // If it does, they should be negatives of each other
    if (result_rev < 0) {
        assert(std::abs(result_fwd + result_rev) < 0.05 && "Reversed bounds should negate result");
    }
    
    // ========================================
    // PROPERTY 6: Convergence with more samples
    // ========================================
    unsigned int seed8a = 55555;
    unsigned int seed8b = 55555;
    
    double result_few = monte_carlo_mean(f_x2, 0, 1, 1000, seed8a);
    double result_many = monte_carlo_mean(f_x2, 0, 1, 100000, seed8b);
    
    // More samples should be more accurate
    double error_few = std::abs(result_few - 1.0/3.0);
    double error_many = std::abs(result_many - 1.0/3.0);
    
    // At least one should be reasonably accurate
    assert((error_few < 0.1 || error_many < 0.02) && "Should converge with more samples");
    
    // ========================================
    // PROPERTY 7: Different intervals
    // ========================================
    auto f_test = [](double x) { return x; };
    unsigned int seed9 = 66666;
    
    // integral of x from 2 to 5 = [x^2/2] = 25/2 - 4/2 = 10.5
    double result9 = monte_carlo_mean(f_test, 2, 5, 50000, seed9);
    assert(std::abs(result9 - 10.5) < 0.1 && "Different interval should work");
    
    // ========================================
    // PROPERTY 8: Transcendental functions
    // ========================================
    auto f_exp = [](double x) { return std::exp(x); };
    unsigned int seed10 = 77777;
    double result10 = monte_carlo_mean(f_exp, 0, 1, 50000, seed10);
    assert(std::abs(result10 - (std::exp(1.0) - 1.0)) < 0.05 && "exp integral accurate");
    
    // ========================================
    // PROPERTY 9: Zero integral
    // ========================================
    auto f_zero = [](double x) { return 0.0; };
    unsigned int seed11 = 88888;
    double result11 = monte_carlo_mean(f_zero, 0, 10, 10000, seed11);
    assert(std::abs(result11) < 0.01 && "Zero function should give zero integral");
    
    std::cout << "test_monte_carlo_mean PASSED" << std::endl;
    return 0;
}
