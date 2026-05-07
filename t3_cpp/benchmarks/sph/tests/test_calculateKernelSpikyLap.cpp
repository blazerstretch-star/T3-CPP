#include "../src/sph_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    // Relative tolerance: 0.1% covers any reasonable floating-point expression
    // of 45/(pi*h^6) regardless of operand order or pow() variant used.
    const float REL_EPS = 0.001f;

    // Test 1: Value for h=0.5
    float h1 = 0.5f;
    float spikyLap1 = calculateKernelSpikyLap(h1);
    float expected1 = 45.0f / ((float)M_PI * std::pow(h1, 6));
    assert(std::abs(spikyLap1 - expected1) < REL_EPS * expected1 && "h=0.5 spiky laplacian mismatch");

    // Test 2: Value for h=1.0
    float h2 = 1.0f;
    float spikyLap2 = calculateKernelSpikyLap(h2);
    float expected2 = 45.0f / ((float)M_PI * std::pow(h2, 6));
    assert(std::abs(spikyLap2 - expected2) < REL_EPS * expected2 && "h=1.0 spiky laplacian mismatch");

    // Test 3: Must be positive
    assert(spikyLap1 > 0 && "Spiky laplacian must be positive");
    assert(spikyLap2 > 0 && "Spiky laplacian must be positive");

    // Test 4: Smaller h gives larger value
    float h3 = 0.1f;
    float spikyLap3 = calculateKernelSpikyLap(h3);
    assert(spikyLap3 > spikyLap1 && "Smaller h must give larger laplacian");

    // Test 5: Larger h gives smaller value
    float h4 = 2.0f;
    float spikyLap4 = calculateKernelSpikyLap(h4);
    assert(spikyLap4 < spikyLap2 && "Larger h must give smaller laplacian");

    // Test 6: Verify h^6 scaling
    float ratio = spikyLap1 / spikyLap2;
    float expected_ratio = std::pow(h2 / h1, 6);
    assert(std::abs(ratio - expected_ratio) < 0.01f && "h^6 scaling violated");

    // Test 7: Opposite sign of gradient — both use same formula magnitude
    float grad = calculateKernelSpikyGrad(h1);
    assert((spikyLap1 > 0 && grad < 0) && "Laplacian and gradient must have opposite signs");
    assert(std::abs(spikyLap1 + grad) < 0.01f && "Magnitude must match within epsilon");

    std::cout << "All calculateKernelSpikyLap tests passed!" << std::endl;
    return 0;
}
