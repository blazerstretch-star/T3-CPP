#include "../src/cppqed_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    const double tol = 1e-10;
    
    // Test 1: cutoff=3
    auto result1 = calculate_mode_annihilation_diagonal(3);
    assert(result1.size() == 2);
    assert(std::abs(result1[0] - dcomp(1.0, 0.0)) < tol); // sqrt(1)
    assert(std::abs(result1[1] - dcomp(std::sqrt(2.0), 0.0)) < tol); // sqrt(2)
    
    // Test 2: cutoff=5
    auto result2 = calculate_mode_annihilation_diagonal(5);
    assert(result2.size() == 4);
    assert(std::abs(result2[0] - dcomp(1.0, 0.0)) < tol);
    assert(std::abs(result2[1] - dcomp(std::sqrt(2.0), 0.0)) < tol);
    assert(std::abs(result2[2] - dcomp(std::sqrt(3.0), 0.0)) < tol);
    assert(std::abs(result2[3] - dcomp(2.0, 0.0)) < tol); // sqrt(4)
    
    // Test 3: cutoff=1 (minimum case)
    auto result3 = calculate_mode_annihilation_diagonal(1);
    assert(result3.size() == 0); // cutoff-1 = 0
    
    // Test 4: cutoff=2
    auto result4 = calculate_mode_annihilation_diagonal(2);
    assert(result4.size() == 1);
    assert(std::abs(result4[0] - dcomp(1.0, 0.0)) < tol); // sqrt(1)
    
    // Test 5: Larger cutoff
    auto result5 = calculate_mode_annihilation_diagonal(10);
    assert(result5.size() == 9);
    assert(std::abs(result5[0] - dcomp(1.0, 0.0)) < tol);           // sqrt(1)
    assert(std::abs(result5[4] - dcomp(std::sqrt(5.0), 0.0)) < tol); // sqrt(5)
    assert(std::abs(result5[8] - dcomp(3.0, 0.0)) < tol);           // sqrt(9)
    
    // Test 6: Verify all elements are real (imaginary part = 0)
    auto result6 = calculate_mode_annihilation_diagonal(7);
    for (const auto& val : result6) {
        assert(std::abs(std::imag(val)) < tol);
    }
    
    // Test 7: Verify all elements are positive
    auto result7 = calculate_mode_annihilation_diagonal(8);
    for (const auto& val : result7) {
        assert(std::real(val) > 0.0);
    }
    
    // Test 8: Verify formula a[i] = sqrt(i+1)
    auto result8 = calculate_mode_annihilation_diagonal(15);
    for (size_t i = 0; i < result8.size(); ++i) {
        double expected = std::sqrt(static_cast<double>(i + 1));
        assert(std::abs(result8[i] - dcomp(expected, 0.0)) < tol);
    }
    
    // Test 9: Verify monotonically increasing
    auto result9 = calculate_mode_annihilation_diagonal(20);
    for (size_t i = 1; i < result9.size(); ++i) {
        assert(std::real(result9[i]) > std::real(result9[i-1]));
    }
    
    // Test 10: Verify number operator property: a†a|n> = n|n>
    // The diagonal elements should satisfy: |a[n]|^2 = n+1
    auto result10 = calculate_mode_annihilation_diagonal(6);
    for (size_t n = 0; n < result10.size(); ++n) {
        double a_n = std::real(result10[n]);  // sqrt(n+1)
        assert(std::abs(a_n * a_n - (n + 1.0)) < tol);
    }
    
    std::cout << "All tests passed for calculate_mode_annihilation_diagonal!" << std::endl;
    return 0;
}
