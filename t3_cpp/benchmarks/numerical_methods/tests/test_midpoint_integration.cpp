#include "../src/numerical_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    using namespace NumericalMethods;
    
    auto f_sin = [](double x) { return std::sin(x); };
    double result1 = midpoint_integration(f_sin, 0, M_PI, 10000);
    assert(std::abs(result1 - 2.0) < 0.0001);
    
    auto f_x = [](double x) { return x; };
    double result2 = midpoint_integration(f_x, 0, 1, 10000);
    assert(std::abs(result2 - 0.5) < 0.00001);
    
    auto f_x2 = [](double x) { return x * x; };
    double result3 = midpoint_integration(f_x2, 0, 1, 10000);
    assert(std::abs(result3 - 1.0/3.0) < 0.0001);
    
    auto f_exp = [](double x) { return std::exp(x); };
    double result4 = midpoint_integration(f_exp, 0, 1, 10000);
    assert(std::abs(result4 - (std::exp(1.0) - 1.0)) < 0.0001);
    
    auto f_const = [](double x) { return 5.0; };
    double result5 = midpoint_integration(f_const, 0, 2, 100);
    assert(std::abs(result5 - 10.0) < 0.0001);
    
    std::cout << "test_midpoint_integration PASSED" << std::endl;
    return 0;
}
