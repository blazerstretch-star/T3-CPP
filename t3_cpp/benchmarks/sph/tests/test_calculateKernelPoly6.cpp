#include "../src/sph_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    // Relative tolerance: 0.1% covers any reasonable floating-point expression
    // of 315/(64*pi*h^9) regardless of operand order or pow() variant used.
    const float REL_EPS = 0.001f;

    // Test 1: Value for h=0.5
    float h1 = 0.5f;
    float poly6_1 = calculateKernelPoly6(h1);
    float expected1 = 315.0f / (64.0f * (float)M_PI * std::pow(h1, 9));
    assert(std::abs(poly6_1 - expected1) < REL_EPS * expected1 && "h=0.5 kernel mismatch");

    // Test 2: Value for h=1.0
    float h2 = 1.0f;
    float poly6_2 = calculateKernelPoly6(h2);
    float expected2 = 315.0f / (64.0f * (float)M_PI * std::pow(h2, 9));
    assert(std::abs(poly6_2 - expected2) < REL_EPS * expected2 && "h=1.0 kernel mismatch");

    // Test 3: Value for h=0.1
    float h3 = 0.1f;
    float poly6_3 = calculateKernelPoly6(h3);
    float expected3 = 315.0f / (64.0f * (float)M_PI * std::pow(h3, 9));
    assert(std::abs(poly6_3 - expected3) < REL_EPS * expected3 && "h=0.1 kernel mismatch");

    // Test 4: Inverse relationship - smaller h gives larger kernel
    assert(poly6_3 > poly6_1 && "Smaller h must give larger kernel");
    assert(poly6_1 > poly6_2 && "h=0.5 must exceed h=1.0");

    // Test 5: Value for h=2.0
    float h4 = 2.0f;
    float poly6_4 = calculateKernelPoly6(h4);
    float expected4 = 315.0f / (64.0f * (float)M_PI * std::pow(h4, 9));
    assert(std::abs(poly6_4 - expected4) < REL_EPS * expected4 && "h=2.0 kernel mismatch");
    assert(poly6_4 < poly6_2 && "Larger h must give smaller kernel");

    // Test 6: All values must be positive
    assert(poly6_1 > 0 && "Kernel must be positive");
    assert(poly6_2 > 0 && "Kernel must be positive");
    assert(poly6_3 > 0 && "Kernel must be positive");
    assert(poly6_4 > 0 && "Kernel must be positive");

    // Test 7: Verify scaling relationship (h^9 in denominator)
    float ratio = poly6_1 / poly6_2;
    float expected_ratio = std::pow(h2 / h1, 9);
    assert(std::abs(ratio - expected_ratio) < 0.01f && "Scaling relationship violated");

    std::cout << "All calculateKernelPoly6 tests passed!" << std::endl;
    return 0;
}
