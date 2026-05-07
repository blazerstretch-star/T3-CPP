#include "../src/sph_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    const float EPSILON = 0.00001f;
    
    // Test 1: Exact calculation verification
    float dist2 = 0.1f;
    float h2 = 0.25f;
    float massPoly6 = 50.0f;
    float contrib = calculateDensityContribution(dist2, h2, massPoly6);
    float expected = massPoly6 * std::pow(h2 - dist2, 3);
    assert(std::abs(contrib - expected) < EPSILON && "Density contribution mismatch");
    
    // Test 2: Different parameters
    float dist2_2 = 0.05f;
    float h2_2 = 0.16f;
    float massPoly6_2 = 100.0f;
    float contrib2 = calculateDensityContribution(dist2_2, h2_2, massPoly6_2);
    float expected2 = massPoly6_2 * std::pow(h2_2 - dist2_2, 3);
    assert(std::abs(contrib2 - expected2) < EPSILON && "Second calculation mismatch");
    
    // Test 3: All contributions must be positive
    assert(contrib > 0 && "Contribution must be positive");
    assert(contrib2 > 0 && "Contribution must be positive");
    
    // Test 4: Closer particles (smaller dist2) give larger contribution
    float contrib_close = calculateDensityContribution(0.01f, 0.25f, 50.0f);
    float contrib_far = calculateDensityContribution(0.2f, 0.25f, 50.0f);
    assert(contrib_close > contrib_far && "Closer particles must contribute more");
    
    // Test 5: Linear scaling with massPoly6
    float contrib_mass1 = calculateDensityContribution(0.1f, 0.25f, 50.0f);
    float contrib_mass2 = calculateDensityContribution(0.1f, 0.25f, 100.0f);
    assert(std::abs(contrib_mass2 / contrib_mass1 - 2.0f) < 0.01f && "Mass scaling violated");
    
    // Test 6: Edge case - particle at boundary (dist2 ≈ h2)
    float contrib_boundary = calculateDensityContribution(0.24f, 0.25f, 50.0f);
    float expected_boundary = 50.0f * std::pow(0.01f, 3);
    assert(std::abs(contrib_boundary - expected_boundary) < EPSILON && "Boundary case mismatch");
    
    // Test 7: Zero distance
    float contrib_zero = calculateDensityContribution(0.0f, 0.25f, 50.0f);
    float expected_zero = 50.0f * std::pow(0.25f, 3);
    assert(std::abs(contrib_zero - expected_zero) < EPSILON && "Zero distance mismatch");

    // Test 8: dist2 == h2 — (h2 - dist2)^3 = 0
    float contrib_at_boundary = calculateDensityContribution(0.25f, 0.25f, 50.0f);
    assert(std::abs(contrib_at_boundary) < EPSILON && "dist2==h2: zero contribution");

    std::cout << "All calculateDensityContribution tests passed!" << std::endl;
    return 0;
}
