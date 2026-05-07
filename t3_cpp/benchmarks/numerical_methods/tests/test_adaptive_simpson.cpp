#include "../src/numerical_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    using namespace NumericalMethods;
    
    auto f_sin = [](double x) { return std::sin(x); };
    double result1 = adaptive_simpson(f_sin, 0, M_PI, 0.0001);
    assert(std::abs(result1 - 2.0) < 0.001);
    
    auto f_x3 = [](double x) { return x * x * x; };
    double result2 = adaptive_simpson(f_x3, 0, 1, 0.0001);
    assert(std::abs(result2 - 0.25) < 0.001);
    
    auto f_cos = [](double x) { return std::cos(x); };
    double result3 = adaptive_simpson(f_cos, 0, M_PI/2, 0.0001);
    assert(std::abs(result3 - 1.0) < 0.001);
    
    std::cout << "test_adaptive_simpson PASSED" << std::endl;
    return 0;
}
