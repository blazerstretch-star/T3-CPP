#include "../src/cppqed_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    const double tol = 1e-9;
    
    // Test 1: Small dimension with complete validation
    auto result1 = fill_hermite_array(3, 1.0);
    assert(result1.size() == 5); // 2*3-1 = 5
    assert(std::abs(result1[0] - 1.0) < tol);  // H_0(x) = 1
    assert(std::abs(result1[1] - 2.0) < tol);  // H_1(1) = 2*1 = 2
    assert(std::abs(result1[2] - 2.0) < tol);  // H_2(1) = 4*1^2 - 2 = 2
    assert(std::abs(result1[3] - -4.0) < tol); // H_3(1) = 8*1^3 - 12*1 = -4
    assert(std::abs(result1[4] - -20.0) < tol); // H_4(1) = 16*1^4 - 48*1^2 + 12 = -20
    
    // Test 2: Zero point - Hermite polynomials at x=0
    auto result2 = fill_hermite_array(2, 0.0);
    assert(result2.size() == 3); // 2*2-1 = 3
    assert(std::abs(result2[0] - 1.0) < tol);  // H_0(0) = 1
    assert(std::abs(result2[1] - 0.0) < tol);  // H_1(0) = 0
    assert(std::abs(result2[2] - -2.0) < tol); // H_2(0) = -2
    
    // Test 3: Larger dimension with validation
    auto result3 = fill_hermite_array(5, 0.5);
    assert(result3.size() == 9); // 2*5-1 = 9
    assert(std::abs(result3[0] - 1.0) < tol);  // H_0(x) = 1
    assert(std::abs(result3[1] - 1.0) < tol);  // H_1(0.5) = 2*0.5 = 1.0
    assert(std::abs(result3[2] - -1.0) < tol); // H_2(0.5) = 4*0.25 - 2 = -1.0
    
    // Test 4: Verify recurrence relation H_{n+1}(x) = 2x*H_n(x) - 2n*H_{n-1}(x)
    double x = 1.5;
    auto result4 = fill_hermite_array(4, x);
    for (size_t n = 1; n < result4.size() - 1; ++n) {
        double recurrence = 2.0 * x * result4[n] - 2.0 * n * result4[n-1];
        assert(std::abs(result4[n+1] - recurrence) < tol);
    }
    
    // Test 5: Negative x value
    auto result5 = fill_hermite_array(3, -1.0);
    assert(result5.size() == 5);
    assert(std::abs(result5[0] - 1.0) < tol);   // H_0(-1) = 1
    assert(std::abs(result5[1] - -2.0) < tol);  // H_1(-1) = -2
    assert(std::abs(result5[2] - 2.0) < tol);   // H_2(-1) = 2 (even function)
    assert(std::abs(result5[3] - 4.0) < tol);   // H_3(-1) = 4 (odd function)
    
    // Test 6: Verify parity - H_n(-x) = (-1)^n * H_n(x)
    double x6 = 0.7;
    auto pos = fill_hermite_array(4, x6);
    auto neg = fill_hermite_array(4, -x6);
    for (size_t n = 0; n < pos.size(); ++n) {
        double sign = (n % 2 == 0) ? 1.0 : -1.0;
        assert(std::abs(neg[n] - sign * pos[n]) < tol);
    }
    
    // Test 7: Minimum dimension
    auto result7 = fill_hermite_array(1, 2.0);
    assert(result7.size() == 1); // 2*1-1 = 1
    assert(std::abs(result7[0] - 1.0) < tol); // H_0(2) = 1
    
    std::cout << "All tests passed for fill_hermite_array!" << std::endl;
    return 0;
}
