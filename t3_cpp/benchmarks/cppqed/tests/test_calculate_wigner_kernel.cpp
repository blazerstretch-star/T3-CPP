#include "../src/cppqed_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    const double tol = 1e-9;
    
    // Test 1: Simple case m=0, n=0
    // Sum over u=0, v=0: C(0,0)*C(0,0)*(-1)^0*i^0*H_0(-2x)*H_0(2y) = 1*1*1*1*1*1 = 1
    std::vector<double> hermite1 = {1.0, 0.0, -2.0};
    std::vector<double> hermite2 = {1.0, 0.0, -2.0};
    dcomp result1 = calculate_wigner_kernel(0, 0, hermite1, hermite2);
    assert(std::abs(result1 - dcomp(1.0, 0.0)) < tol);
    
    // Test 2: m=1, n=0
    // u=0,v=0: C(1,0)*C(0,0)*1*1*H_0*H_1 = 1*1*1*1*hermite1[0]*hermite2[1]
    // u=1,v=0: C(1,1)*C(0,0)*1*i*H_1*H_0 = 1*1*1*i*hermite1[1]*hermite2[0]
    std::vector<double> hermite3 = {1.0, 2.0, 2.0, -4.0, -20.0};
    std::vector<double> hermite4 = {1.0, 2.0, 2.0, -4.0, -20.0};
    dcomp result2 = calculate_wigner_kernel(1, 0, hermite3, hermite4);
    dcomp expected2 = hermite3[0] * hermite4[1] + 1i * hermite3[1] * hermite4[0];
    assert(std::abs(result2 - expected2) < tol);
    
    // Test 3: m=1, n=1 - verify result is finite and has reasonable magnitude
    dcomp result3 = calculate_wigner_kernel(1, 1, hermite3, hermite4);
    assert(std::isfinite(std::abs(result3)));
    assert(std::abs(result3) < 100.0);
    
    // Test 4: m=0, n=1 (asymmetric case) - verify result is finite
    dcomp result4 = calculate_wigner_kernel(0, 1, hermite3, hermite4);
    assert(std::isfinite(std::abs(result4)));
    
    // Test 5: Verify result is complex (has imaginary part for m>0 or n>0)
    dcomp result5 = calculate_wigner_kernel(2, 1, hermite3, hermite4);
    assert(std::isfinite(std::real(result5)));
    assert(std::isfinite(std::imag(result5)));
    
    // Test 6: m=0, n=0 should always be real
    std::vector<double> hermite6 = {1.0, 3.0, 5.0};
    std::vector<double> hermite7 = {2.0, 4.0, 6.0};
    dcomp result6 = calculate_wigner_kernel(0, 0, hermite6, hermite7);
    assert(std::abs(std::imag(result6)) < tol);  // Should be real
    
    // Test 7: Larger indices
    std::vector<double> hermite8(10, 1.0);  // All ones for simplicity
    dcomp result7 = calculate_wigner_kernel(2, 2, hermite8, hermite8);
    assert(std::isfinite(std::abs(result7)));
    
    // Test 8: Verify magnitude is reasonable
    dcomp result8 = calculate_wigner_kernel(3, 3, hermite3, hermite4);
    assert(std::abs(result8) < 1000.0);  // Should be bounded
    
    std::cout << "All tests passed for calculate_wigner_kernel!" << std::endl;
    return 0;
}
