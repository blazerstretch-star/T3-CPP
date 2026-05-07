#include "../src/md_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    const double EPSILON = 1e-6;
    double r_min = std::pow(2.0, 1.0/6.0);
    
    // Test 1: Force at r = 2^(1/6) * sigma (minimum, should be ~zero)
    vec3 r1(r_min, 0.0, 0.0);
    vec3 force1 = calculateLJForce(r1, 1.0, 1.0);
    assert(std::abs(force1.x) < EPSILON);
    
    // Test 2: Repulsive at r < r_min
    vec3 r2(0.9, 0.0, 0.0);
    vec3 force2 = calculateLJForce(r2, 1.0, 1.0);
    assert(force2.x > 0); // Points along r (repulsive)
    
    // Test 3: Attractive at r > r_min  
    vec3 r3(1.5, 0.0, 0.0);
    vec3 force3 = calculateLJForce(r3, 1.0, 1.0);
    assert(force3.x < 0); // Points opposite to r (attractive)
    
    // Test 4: Force direction along vector
    vec3 r4(1.5, 1.5, 0.0);
    vec3 force4 = calculateLJForce(r4, 1.0, 1.0);
    double ratio = force4.y / force4.x;
    assert(std::abs(ratio - 1.0) < EPSILON);
    
    std::cout << "All tests passed for calculateLJForce!" << std::endl;
    return 0;
}
