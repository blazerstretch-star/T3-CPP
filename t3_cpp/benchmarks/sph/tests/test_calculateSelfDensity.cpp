#include "../src/sph_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    const float EPSILON = 0.00001f;
    
    // Test 1: Exact calculation
    float mass = 1.0f;
    float poly6 = 100.0f;
    float h = 0.5f;
    float selfDens = calculateSelfDensity(mass, poly6, h);
    float expected = mass * poly6 * std::pow(h * h, 3);
    assert(std::abs(selfDens - expected) < EPSILON && "Self density calculation mismatch");
    
    // Test 2: Different parameters
    float selfDens2 = calculateSelfDensity(2.0f, 50.0f, 0.5f);
    float expected2 = 2.0f * 50.0f * std::pow(0.25f, 3);
    assert(std::abs(selfDens2 - expected2) < EPSILON && "Second calculation mismatch");
    
    // Test 3: Must be positive
    assert(selfDens > 0 && "Self density must be positive");
    assert(selfDens2 > 0 && "Self density must be positive");
    
    // Test 4: Linear mass scaling
    float selfDens_m1 = calculateSelfDensity(1.0f, 100.0f, 0.5f);
    float selfDens_m2 = calculateSelfDensity(2.0f, 100.0f, 0.5f);
    assert(std::abs(selfDens_m2 / selfDens_m1 - 2.0f) < 0.01f && "Mass scaling violated");
    
    // Test 5: Linear poly6 scaling
    float selfDens_p1 = calculateSelfDensity(1.0f, 50.0f, 0.5f);
    float selfDens_p2 = calculateSelfDensity(1.0f, 100.0f, 0.5f);
    assert(std::abs(selfDens_p2 / selfDens_p1 - 2.0f) < 0.01f && "Poly6 scaling violated");
    
    // Test 6: h^6 scaling (since (h^2)^3 = h^6)
    float selfDens_h1 = calculateSelfDensity(1.0f, 100.0f, 0.5f);
    float selfDens_h2 = calculateSelfDensity(1.0f, 100.0f, 1.0f);
    float ratio = selfDens_h2 / selfDens_h1;
    float expected_ratio = std::pow(1.0f / 0.5f, 6);
    assert(std::abs(ratio - expected_ratio) < 0.01f && "h^6 scaling violated");
    
    // Test 7: Zero mass edge case
    float selfDens_zero = calculateSelfDensity(0.0f, 100.0f, 0.5f);
    assert(selfDens_zero == 0.0f && "Zero mass must give zero density");
    
    std::cout << "All calculateSelfDensity tests passed!" << std::endl;
    return 0;
}
