#include "../src/md_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    const double EPSILON = 1e-10;
    double boxSize = 10.0;
    
    // Test 1: Simple distance (no wrapping)
    vec3 r1(2.0, 2.0, 2.0);
    vec3 r2(5.0, 5.0, 5.0);
    vec3 dr1 = minimumImageDistance(r1, r2, boxSize);
    assert(std::abs(dr1.x - 3.0) < EPSILON);
    assert(std::abs(dr1.y - 3.0) < EPSILON);
    assert(std::abs(dr1.z - 3.0) < EPSILON);
    
    // Test 2: Distance across periodic boundary (x-direction)
    vec3 r3(1.0, 5.0, 5.0);
    vec3 r4(9.0, 5.0, 5.0);
    vec3 dr2 = minimumImageDistance(r3, r4, boxSize);
    assert(std::abs(dr2.x - (-2.0)) < EPSILON); // Shorter to go backwards
    assert(std::abs(dr2.y - 0.0) < EPSILON);
    assert(std::abs(dr2.z - 0.0) < EPSILON);
    
    // Test 3: Distance across periodic boundary (reverse)
    vec3 dr3 = minimumImageDistance(r4, r3, boxSize);
    assert(std::abs(dr3.x - 2.0) < EPSILON); // Opposite direction
    assert(std::abs(dr3.y - 0.0) < EPSILON);
    assert(std::abs(dr3.z - 0.0) < EPSILON);
    
    // Test 4: Multiple periodic boundaries
    vec3 r5(1.0, 1.0, 1.0);
    vec3 r6(9.0, 9.0, 9.0);
    vec3 dr4 = minimumImageDistance(r5, r6, boxSize);
    assert(std::abs(dr4.x - (-2.0)) < EPSILON);
    assert(std::abs(dr4.y - (-2.0)) < EPSILON);
    assert(std::abs(dr4.z - (-2.0)) < EPSILON);
    
    // Test 5: Exactly at half box size
    vec3 r7(0.0, 0.0, 0.0);
    vec3 r8(5.0, 0.0, 0.0);
    vec3 dr5 = minimumImageDistance(r7, r8, boxSize);
    assert(std::abs(dr5.x - 5.0) < EPSILON || std::abs(dr5.x + 5.0) < EPSILON);
    
    std::cout << "All tests passed for minimumImageDistance!" << std::endl;
    return 0;
}
