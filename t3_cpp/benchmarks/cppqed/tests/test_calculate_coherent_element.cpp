#include "../src/cppqed_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <boost/math/special_functions/factorials.hpp>

int main() {
    const double tol = 1e-9;
    
    // Test 1: n=0 should always return 1
    dcomp result1 = calculate_coherent_element(0, dcomp(1.0, 0.5));
    assert(std::abs(result1 - dcomp(1.0, 0.0)) < tol);
    
    // Test 2: n=1, alpha=2.0
    dcomp alpha2(2.0, 0.0);
    dcomp result2 = calculate_coherent_element(1, alpha2);
    assert(std::abs(result2 - alpha2) < tol); // alpha^1 / sqrt(1!) = alpha
    
    // Test 3: n=2, alpha=1.0
    dcomp alpha3(1.0, 0.0);
    dcomp result3 = calculate_coherent_element(2, alpha3);
    dcomp expected3 = dcomp(1.0, 0.0) / std::sqrt(2.0); // 1^2 / sqrt(2!)
    assert(std::abs(result3 - expected3) < tol);
    
    // Test 4: n=3, alpha=2.0
    dcomp result4 = calculate_coherent_element(3, dcomp(2.0, 0.0));
    dcomp expected4 = std::pow(2.0, 3) / std::sqrt(6.0); // 8 / sqrt(6)
    assert(std::abs(result4 - expected4) < tol);
    
    // Test 5: Complex alpha
    dcomp alpha5(1.0, 1.0);
    dcomp result5 = calculate_coherent_element(2, alpha5);
    dcomp expected5 = std::pow(alpha5, 2) / std::sqrt(2.0);
    assert(std::abs(result5 - expected5) < tol);
    
    // Test 6: Boundary case - just below max_factorial
    using boost::math::max_factorial;
    unsigned long n6 = max_factorial<double>::value - 1;
    dcomp result6 = calculate_coherent_element(n6, dcomp(2.0, 0.0));
    assert(std::isfinite(std::abs(result6)));
    
    // Test 7: Boundary case - at max_factorial (uses approximation)
    unsigned long n7 = max_factorial<double>::value;
    dcomp result7 = calculate_coherent_element(n7, dcomp(2.0, 0.0));
    assert(std::isfinite(std::abs(result7)));
    
    // Test 8: Large n with approximation
    dcomp alpha8(10.0, 0.0);
    dcomp result8 = calculate_coherent_element(200, alpha8);
    // Stirling: alpha^n / sqrt(n!) ≈ (2πn)^(-1/4) * (alpha*e/sqrt(n))^n
    double n8 = 200.0;
    dcomp expected8 = std::pow(2.0 * n8 * std::numbers::pi, -0.25) * 
                      std::pow(alpha8 / std::sqrt(n8 / std::numbers::e), n8);
    assert(std::abs(result8 - expected8) < std::abs(expected8) * 1e-6); // Relative error
    
    // Test 9: Verify n=0 with different alphas
    assert(std::abs(calculate_coherent_element(0, dcomp(0.0, 0.0)) - dcomp(1.0, 0.0)) < tol);
    assert(std::abs(calculate_coherent_element(0, dcomp(100.0, 50.0)) - dcomp(1.0, 0.0)) < tol);
    
    // Test 10: Small alpha, various n
    dcomp alpha10(0.5, 0.0);
    for (unsigned long n = 1; n <= 5; ++n) {
        dcomp result = calculate_coherent_element(n, alpha10);
        dcomp expected = std::pow(alpha10, n) / std::sqrt(boost::math::factorial<double>(n));
        assert(std::abs(result - expected) < tol);
    }
    
    // Test 11: Verify magnitude decreases for |alpha| < 1
    dcomp alpha11(0.5, 0.0);
    double mag1 = std::abs(calculate_coherent_element(5, alpha11));
    double mag2 = std::abs(calculate_coherent_element(10, alpha11));
    assert(mag2 < mag1); // Should decrease for small alpha
    
    // Test 12: Pure imaginary alpha
    dcomp alpha12(0.0, 2.0);
    dcomp result12 = calculate_coherent_element(2, alpha12);
    dcomp expected12 = std::pow(alpha12, 2) / std::sqrt(2.0);
    assert(std::abs(result12 - expected12) < tol);
    
    // Test 13: Negative real alpha
    dcomp alpha13(-1.5, 0.0);
    dcomp result13 = calculate_coherent_element(3, alpha13);
    dcomp expected13 = std::pow(alpha13, 3) / std::sqrt(6.0);
    assert(std::abs(result13 - expected13) < tol);
    
    // Test 14: Verify phase for complex alpha
    dcomp alpha14(1.0, 1.0);
    dcomp result14 = calculate_coherent_element(4, alpha14);
    // alpha^4 = (1+i)^4 = ((1+i)^2)^2 = (2i)^2 = -4
    dcomp expected14 = dcomp(-4.0, 0.0) / std::sqrt(24.0);
    assert(std::abs(result14 - expected14) < tol);
    
    std::cout << "All tests passed for calculate_coherent_element!" << std::endl;
    return 0;
}
