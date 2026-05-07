#include "../src/numerical_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    using namespace NumericalMethods;
    
    // Test 1: f(x) = x^2 - 4, root at x=2
    auto f1 = [](double x) { return x * x - 4; };
    double root1 = bisection_root_finder(f1, 0, 3, 1e-8, 100);
    assert(std::abs(root1 - 2.0) < 1e-7);
    
    // Test 2: f(x) = x - 1, root at x=1
    auto f2 = [](double x) { return x - 1; };
    double root2 = bisection_root_finder(f2, 0, 2, 1e-8, 100);
    assert(std::abs(root2 - 1.0) < 1e-7);
    
    // Test 3: f(x) = x^3 - 2, root at x=2^(1/3)
    auto f3 = [](double x) { return x * x * x - 2; };
    double root3 = bisection_root_finder(f3, 1, 2, 1e-8, 100);
    assert(std::abs(root3 - std::pow(2.0, 1.0/3.0)) < 1e-7);
    
    // Test 4: f(x) = cos(x), root at x=pi/2
    auto f4 = [](double x) { return std::cos(x); };
    double root4 = bisection_root_finder(f4, 0, M_PI, 1e-8, 100);
    assert(std::abs(root4 - M_PI/2.0) < 1e-7);
    
    // Test 5: Reversed bounds (should still work)
    double root5 = bisection_root_finder(f1, 3, 0, 1e-8, 100);
    assert(std::abs(root5 - 2.0) < 1e-7);
    
    std::cout << "test_bisection_root_finder_strict PASSED" << std::endl;
    return 0;
}
