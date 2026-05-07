#include "../src/cppqed_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    const double tol = 1e-9;
    
    // Test 1: MFT_SIN (type 0) - sin(π/2) = 1
    dcomp result1 = calculate_mode_function(0, PI / 2.0);
    assert(std::abs(result1 - dcomp(1.0, 0.0)) < tol);
    
    // Test 2: MFT_COS (type 1) - cos(0) = 1
    dcomp result2 = calculate_mode_function(1, 0.0);
    assert(std::abs(result2 - dcomp(1.0, 0.0)) < tol);
    
    // Test 3: MFT_PLUS (type 2) - exp(iπ) = -1
    dcomp result3 = calculate_mode_function(2, PI);
    assert(std::abs(result3 - dcomp(-1.0, 0.0)) < tol);
    
    // Test 4: MFT_MINUS (type 3) - exp(-iπ) = -1
    dcomp result4 = calculate_mode_function(3, PI);
    assert(std::abs(result4 - dcomp(-1.0, 0.0)) < tol);
    
    // Test 5: sin(0) = 0
    dcomp result5 = calculate_mode_function(0, 0.0);
    assert(std::abs(result5) < tol);
    
    // Test 6: cos(π/2) = 0
    dcomp result6 = calculate_mode_function(1, PI / 2.0);
    assert(std::abs(result6) < tol);
    
    // Test 7: sin(π) = 0
    dcomp result7 = calculate_mode_function(0, PI);
    assert(std::abs(result7) < tol);
    
    // Test 8: cos(π) = -1
    dcomp result8 = calculate_mode_function(1, PI);
    assert(std::abs(result8 - dcomp(-1.0, 0.0)) < tol);
    
    // Test 9: exp(i*π/2) = i
    dcomp result9 = calculate_mode_function(2, PI / 2.0);
    assert(std::abs(result9 - dcomp(0.0, 1.0)) < tol);
    
    // Test 10: exp(-i*π/2) = -i
    dcomp result10 = calculate_mode_function(3, PI / 2.0);
    assert(std::abs(result10 - dcomp(0.0, -1.0)) < tol);
    
    // Test 11: Negative x - sin(-x) = -sin(x)
    dcomp result11a = calculate_mode_function(0, PI / 4.0);
    dcomp result11b = calculate_mode_function(0, -PI / 4.0);
    assert(std::abs(result11a + result11b) < tol);
    
    // Test 12: Negative x - cos(-x) = cos(x)
    dcomp result12a = calculate_mode_function(1, PI / 3.0);
    dcomp result12b = calculate_mode_function(1, -PI / 3.0);
    assert(std::abs(result12a - result12b) < tol);
    
    // Test 13: exp(i*2π) = 1 (periodicity)
    dcomp result13 = calculate_mode_function(2, 2.0 * PI);
    assert(std::abs(result13 - dcomp(1.0, 0.0)) < tol);
    
    // Test 14: exp(-i*2π) = 1 (periodicity)
    dcomp result14 = calculate_mode_function(3, 2.0 * PI);
    assert(std::abs(result14 - dcomp(1.0, 0.0)) < tol);
    
    // Test 15: Arbitrary x value - sin(1.5)
    dcomp result15 = calculate_mode_function(0, 1.5);
    assert(std::abs(result15 - dcomp(std::sin(1.5), 0.0)) < tol);
    
    // Test 16: Arbitrary x value - cos(2.3)
    dcomp result16 = calculate_mode_function(1, 2.3);
    assert(std::abs(result16 - dcomp(std::cos(2.3), 0.0)) < tol);
    
    // Test 17: Invalid type (should default to exp(-ix))
    dcomp result17 = calculate_mode_function(4, PI / 2.0);
    dcomp expected17 = std::exp(-1i * PI / 2.0);
    assert(std::abs(result17 - expected17) < tol);
    
    // Test 18: Invalid negative type (should default to exp(-ix))
    dcomp result18 = calculate_mode_function(-1, PI);
    dcomp expected18 = std::exp(-1i * PI);
    assert(std::abs(result18 - expected18) < tol);
    
    // Test 19: Large type number (should default to exp(-ix))
    dcomp result19 = calculate_mode_function(100, PI / 4.0);
    dcomp expected19 = std::exp(-1i * PI / 4.0);
    assert(std::abs(result19 - expected19) < tol);
    
    // Test 20: Verify Euler's formula: exp(ix) = cos(x) + i*sin(x)
    double x20 = 1.234;
    dcomp exp_plus = calculate_mode_function(2, x20);
    dcomp cos_val = calculate_mode_function(1, x20);
    dcomp sin_val = calculate_mode_function(0, x20);
    dcomp euler = cos_val + 1i * sin_val;
    assert(std::abs(exp_plus - euler) < tol);
    
    // Test 21: Verify exp(ix) * exp(-ix) = 1
    double x21 = 2.5;
    dcomp plus = calculate_mode_function(2, x21);
    dcomp minus = calculate_mode_function(3, x21);
    assert(std::abs(plus * minus - dcomp(1.0, 0.0)) < tol);
    
    // Test 22: Large x value
    dcomp result22 = calculate_mode_function(0, 100.0);
    assert(std::isfinite(std::abs(result22)));
    assert(std::abs(result22) <= 1.0); // sin is bounded
    
    std::cout << "All tests passed for calculate_mode_function!" << std::endl;
    return 0;
}
