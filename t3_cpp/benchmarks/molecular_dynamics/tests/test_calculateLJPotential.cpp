#include "../src/md_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    const double EPSILON = 1e-6;
    
    // Test 1: Standard LJ potential at r = sigma
    vec3 r1(1.0, 0.0, 0.0);
    double pot1 = calculateLJPotential(r1, 1.0, 1.0);
    assert(std::abs(pot1 - 0.0) < EPSILON);
    
    // Test 2: LJ potential at r = 2^(1/6) * sigma (minimum)
    double r_min = std::pow(2.0, 1.0/6.0);
    vec3 r2(r_min, 0.0, 0.0);
    double pot2 = calculateLJPotential(r2, 1.0, 1.0);
    assert(std::abs(pot2 + 1.0) < EPSILON);
    
    // Test 3: LJ potential at larger distance
    vec3 r3(2.0, 0.0, 0.0);
    double pot3 = calculateLJPotential(r3, 1.0, 1.0);
    double expected3 = 4.0 * (std::pow(0.5, 12) - std::pow(0.5, 6));
    assert(std::abs(pot3 - expected3) < EPSILON);
    
    // Test 4: Different epsilon and sigma
    vec3 r4(2.0, 0.0, 0.0);
    double pot4 = calculateLJPotential(r4, 2.0, 1.5);
    double sigma_over_r = 1.5 / 2.0;
    double expected4 = 4.0 * 2.0 * (std::pow(sigma_over_r, 12) - std::pow(sigma_over_r, 6));
    assert(std::abs(pot4 - expected4) < EPSILON);
    
    std::cout << "All tests passed for calculateLJPotential!" << std::endl;
    return 0;
}
