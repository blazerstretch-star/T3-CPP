#include "../src/sph_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    // Relative tolerance: 0.1% covers any reasonable floating-point expression
    // of -45/(pi*h^6) regardless of operand order or pow() variant used.
    const float REL_EPS = 0.001f;

    // Test 1: Value for h=0.5
    float h1 = 0.5f;
    float spikyGrad1 = calculateKernelSpikyGrad(h1);
    float expected1 = -45.0f / ((float)M_PI * std::pow(h1, 6));
    assert(std::abs(spikyGrad1 - expected1) < REL_EPS * std::abs(expected1) && "h=0.5 spiky gradient mismatch");

    // Test 2: Value for h=1.0
    float h2 = 1.0f;
    float spikyGrad2 = calculateKernelSpikyGrad(h2);
    float expected2 = -45.0f / ((float)M_PI * std::pow(h2, 6));
    assert(std::abs(spikyGrad2 - expected2) < REL_EPS * std::abs(expected2) && "h=1.0 spiky gradient mismatch");

    // Test 3: Must be negative
    assert(spikyGrad1 < 0 && "Spiky gradient must be negative");
    assert(spikyGrad2 < 0 && "Spiky gradient must be negative");

    // Test 4: Smaller h gives larger magnitude
    float h3 = 0.1f;
    float spikyGrad3 = calculateKernelSpikyGrad(h3);
    assert(spikyGrad3 < spikyGrad1 && "Smaller h must have larger negative magnitude");

    // Test 5: Larger h gives smaller magnitude
    float h4 = 2.0f;
    float spikyGrad4 = calculateKernelSpikyGrad(h4);
    assert(spikyGrad4 > spikyGrad2 && "Larger h must have smaller negative magnitude");

    // Test 6: Verify h^6 scaling
    float ratio = spikyGrad1 / spikyGrad2;
    float expected_ratio = std::pow(h2 / h1, 6);
    assert(std::abs(ratio - expected_ratio) < 0.01f && "h^6 scaling violated");

    // Test 7: Value for h=0.25
    float h5 = 0.25f;
    float spikyGrad5 = calculateKernelSpikyGrad(h5);
    float expected5 = -45.0f / ((float)M_PI * std::pow(h5, 6));
    assert(std::abs(spikyGrad5 - expected5) < REL_EPS * std::abs(expected5) && "h=0.25 mismatch");

    std::cout << "All calculateKernelSpikyGrad tests passed!" << std::endl;
    return 0;
}
