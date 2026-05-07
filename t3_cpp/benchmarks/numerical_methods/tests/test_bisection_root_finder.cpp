#include "../src/numerical_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    using namespace NumericalMethods;
    
    auto f1 = [](double x) { return x * x - 4; };
    double root1 = bisection_root_finder(f1, 0, 3, 1e-10, 1000);
    assert(std::abs(root1 - 2.0) < 1e-8);
    
    auto f2 = [](double x) { return x - 1; };
    double root2 = bisection_root_finder(f2, 0, 2, 1e-10, 1000);
    assert(std::abs(root2 - 1.0) < 1e-8);
    
    auto f3 = [](double x) { return x * x * x - 8; };
    double root3 = bisection_root_finder(f3, 0, 3, 1e-10, 1000);
    assert(std::abs(root3 - 2.0) < 1e-8);
    
    auto f4 = [](double x) { return std::cos(x); };
    double root4 = bisection_root_finder(f4, 0, M_PI, 1e-10, 1000);
    assert(std::abs(root4 - M_PI/2.0) < 1e-8);
    
    auto f5 = [](double x) { return std::exp(x) - 2; };
    double root5 = bisection_root_finder(f5, 0, 2, 1e-10, 1000);
    assert(std::abs(root5 - std::log(2.0)) < 1e-8);
    
    std::cout << "test_bisection_root_finder PASSED" << std::endl;
    return 0;
}
