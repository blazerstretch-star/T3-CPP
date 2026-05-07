#include "../src/cppqed_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    const double tol = 1e-10;
    
    // Test 1: Spin-1/2 (twoS=1)
    auto result1 = calculate_spin_plus_diagonal(1, 2);
    assert(result1.size() == 1); // dim-1
    assert(std::abs(result1[0] - 1.0) < tol); // sqrt((1-0)*(0+1)) = 1
    
    // Test 2: Spin-1 (twoS=2)
    auto result2 = calculate_spin_plus_diagonal(2, 3);
    assert(result2.size() == 2); // dim-1
    assert(std::abs(result2[0] - std::sqrt(2.0)) < tol); // sqrt((2-0)*(0+1)) = sqrt(2)
    assert(std::abs(result2[1] - std::sqrt(2.0)) < tol); // sqrt((2-1)*(1+1)) = sqrt(2)
    
    // Test 3: Spin-3/2 (twoS=3)
    auto result3 = calculate_spin_plus_diagonal(3, 4);
    assert(result3.size() == 3); // dim-1
    assert(std::abs(result3[0] - std::sqrt(3.0)) < tol); // sqrt((3-0)*(0+1)) = sqrt(3)
    assert(std::abs(result3[1] - 2.0) < tol);            // sqrt((3-1)*(1+1)) = 2
    assert(std::abs(result3[2] - std::sqrt(3.0)) < tol); // sqrt((3-2)*(2+1)) = sqrt(3)
    
    // Test 4: dim=0 should default to twoS+1
    auto result4 = calculate_spin_plus_diagonal(2, 0);
    assert(result4.size() == 2); // twoS+1-1 = 2
    assert(std::abs(result4[0] - std::sqrt(2.0)) < tol);
    assert(std::abs(result4[1] - std::sqrt(2.0)) < tol);
    
    // Test 5: dim > twoS+1 should clamp to twoS+1
    auto result5 = calculate_spin_plus_diagonal(2, 10);
    assert(result5.size() == 2); // Should be clamped to twoS+1-1 = 2
    assert(std::abs(result5[0] - std::sqrt(2.0)) < tol);
    assert(std::abs(result5[1] - std::sqrt(2.0)) < tol);
    
    // Test 6: Truncated dimension (dim < twoS+1)
    auto result6 = calculate_spin_plus_diagonal(4, 3);
    assert(result6.size() == 2); // dim-1 = 2
    assert(std::abs(result6[0] - 2.0) < tol);            // sqrt((4-0)*(0+1)) = 2
    assert(std::abs(result6[1] - std::sqrt(6.0)) < tol); // sqrt((4-1)*(1+1)) = sqrt(6)
    
    // Test 7: Spin-2 (twoS=4)
    auto result7 = calculate_spin_plus_diagonal(4, 5);
    assert(result7.size() == 4);
    assert(std::abs(result7[0] - 2.0) < tol);            // sqrt((4-0)*(0+1)) = 2
    assert(std::abs(result7[1] - std::sqrt(6.0)) < tol); // sqrt((4-1)*(1+1)) = sqrt(6)
    assert(std::abs(result7[2] - std::sqrt(6.0)) < tol); // sqrt((4-2)*(2+1)) = sqrt(6)
    assert(std::abs(result7[3] - 2.0) < tol);            // sqrt((4-3)*(3+1)) = 2
    
    // Test 8: Verify all elements are positive
    auto result8 = calculate_spin_plus_diagonal(6, 7);
    for (const auto& val : result8) {
        assert(val > 0.0);
    }
    
    // Test 9: Verify ladder operator property: elements should satisfy
    // s_plus[i] = sqrt((s-m)(s+m+1)) where s = twoS/2, m = -s+i
    size_t twoS9 = 3;
    auto result9 = calculate_spin_plus_diagonal(twoS9, twoS9 + 1);
    double s = twoS9 / 2.0;
    for (size_t i = 0; i < result9.size(); ++i) {
        double m = -s + i;
        double expected = std::sqrt((s - m) * (s + m + 1.0));
        assert(std::abs(result9[i] - expected) < tol);
    }
    
    // Test 10: Minimum case twoS=0 (spin-0, no angular momentum)
    auto result10 = calculate_spin_plus_diagonal(0, 1);
    assert(result10.size() == 0); // dim-1 = 0, no transitions
    
    // Test 11: Large spin value
    auto result11 = calculate_spin_plus_diagonal(10, 11);
    assert(result11.size() == 10);
    assert(std::abs(result11[0] - std::sqrt(10.0)) < tol); // sqrt((10-0)*(0+1))
    assert(std::abs(result11[9] - std::sqrt(10.0)) < tol); // sqrt((10-9)*(9+1))
    
    std::cout << "All tests passed for calculate_spin_plus_diagonal!" << std::endl;
    return 0;
}
