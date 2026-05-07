#include "../src/cppqed_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    const double tol = 1e-9;  // Consistent tolerance for numerical precision
    
    // Test 1: Basic case n=0, k=0, r=1.0
    // Formula: (-1)^n/π * sqrt(n!/(n+k)!) * exp(-2*r^2) * (2*r)^k * L_n^k(4*r^2)
    // For n=0, k=0, r=1.0: 1/π * exp(-2) * 1 * L_0^0(4) = exp(-2)/π (L_0^0(x)=1)
    double result1 = calculate_wigner_weight(0, 1.0, 0);
    double expected1 = std::exp(-2.0) / PI;
    assert(std::abs(result1 - expected1) < tol);
    
    // Test 2: n=1, k=0, r=1.0
    // (-1)^1/π * exp(-2) * L_1^0(4) = -exp(-2)/π * (1-4) = 3*exp(-2)/π
    double result2 = calculate_wigner_weight(1, 1.0, 0);
    double expected2 = 3.0 * std::exp(-2.0) / PI;
    assert(std::abs(result2 - expected2) < tol);
    
    // Test 3: n=0, k=1, r=0.5
    // Formula: 1/π * sqrt(1/1!) * exp(-2*0.25) * 1.0 * L_0^1(1.0)
    // L_0^1(x) = 1, so: exp(-0.5)/π * 1.0 = exp(-0.5)/π
    double result3 = calculate_wigner_weight(0, 0.5, 1);
    double expected3 = std::exp(-0.5) / PI;
    assert(std::abs(result3 - expected3) < tol);
    
    // Test 4: Edge case r=0
    // For r=0: exp(0) * 0^k * L_n^k(0)
    // When k=0: 1/π * 1 * 1 * L_n^0(0)
    double result4 = calculate_wigner_weight(0, 0.0, 0);
    double expected4 = 1.0 / PI;  // L_0^0(0) = 1
    assert(std::abs(result4 - expected4) < tol);
    
    // Test 5: Verify result is finite and reasonable magnitude
    double result5 = calculate_wigner_weight(3, 1.5, 2);
    assert(std::isfinite(result5));
    assert(std::abs(result5) < 10.0);  // Should be bounded
    
    // Test 6: Another parameter combination
    double result6 = calculate_wigner_weight(2, 0.5, 1);
    assert(std::isfinite(result6));
    assert(std::abs(result6) < 10.0);
    
    // Test 7: Larger n value
    double result7 = calculate_wigner_weight(5, 1.0, 0);
    assert(std::isfinite(result7));
    
    // Test 8: Larger k value
    double result8 = calculate_wigner_weight(0, 1.0, 3);
    assert(std::isfinite(result8));
    assert(result8 > 0);  // Should be positive for n=0 (even)
    
    std::cout << "All tests passed for calculate_wigner_weight!" << std::endl;
    return 0;
}
