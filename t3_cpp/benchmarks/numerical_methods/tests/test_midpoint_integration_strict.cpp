#include "../src/numerical_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    using namespace NumericalMethods;
    
    // Test 1: integral of sin(x) from 0 to pi should be 2
    auto f_sin = [](double x) { return std::sin(x); };
    double result1 = midpoint_integration(f_sin, 0, M_PI, 10000);
    assert(std::abs(result1 - 2.0) < 0.0001);  // 0.01% error
    
    // Test 2: integral of x from 0 to 1 should be 0.5
    auto f_x = [](double x) { return x; };
    double result2 = midpoint_integration(f_x, 0, 1, 1000);
    assert(std::abs(result2 - 0.5) < 0.0001);
    
    // Test 3: integral of x^2 from 0 to 1 should be 1/3
    auto f_x2 = [](double x) { return x * x; };
    double result3 = midpoint_integration(f_x2, 0, 1, 1000);
    assert(std::abs(result3 - 1.0/3.0) < 0.001);
    
    // Test 4: integral of e^x from 0 to 1 should be e-1
    auto f_exp = [](double x) { return std::exp(x); };
    double result4 = midpoint_integration(f_exp, 0, 1, 1000);
    assert(std::abs(result4 - (std::exp(1.0) - 1.0)) < 0.001);
    
    // Test 5: Reversed bounds (should give negative result)
    double result5 = midpoint_integration(f_x, 1, 0, 1000);
    assert(std::abs(result5 + 0.5) < 0.0001);  // Should be -0.5
    
    // Test 6: Constant function
    auto f_const = [](double x) { return 5.0; };
    double result6 = midpoint_integration(f_const, 0, 2, 100);
    assert(std::abs(result6 - 10.0) < 0.0001);  // 5 * 2 = 10
    
    std::cout << "test_midpoint_integration_strict PASSED" << std::endl;
    return 0;
}
