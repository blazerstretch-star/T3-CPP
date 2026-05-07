#include "../src/md_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    const double EPSILON = 1e-10;
    double boxSize = 10.0;
    
    // Test 1: Position inside box (no change)
    vec3 pos1(5.0, 5.0, 5.0);
    vec3 result1 = applyPeriodicBoundary(pos1, boxSize);
    assert(std::abs(result1.x - 5.0) < EPSILON);
    assert(std::abs(result1.y - 5.0) < EPSILON);
    assert(std::abs(result1.z - 5.0) < EPSILON);
    
    // Test 2: Position beyond upper boundary
    vec3 pos2(12.0, 5.0, 5.0);
    vec3 result2 = applyPeriodicBoundary(pos2, boxSize);
    assert(std::abs(result2.x - 2.0) < EPSILON);
    assert(std::abs(result2.y - 5.0) < EPSILON);
    assert(std::abs(result2.z - 5.0) < EPSILON);
    
    // Test 3: Position below lower boundary
    vec3 pos3(-2.0, 5.0, 5.0);
    vec3 result3 = applyPeriodicBoundary(pos3, boxSize);
    assert(std::abs(result3.x - 8.0) < EPSILON);
    assert(std::abs(result3.y - 5.0) < EPSILON);
    assert(std::abs(result3.z - 5.0) < EPSILON);
    
    // Test 4: Multiple boundaries
    vec3 pos4(-1.0, 11.0, -3.0);
    vec3 result4 = applyPeriodicBoundary(pos4, boxSize);
    assert(std::abs(result4.x - 9.0) < EPSILON);
    assert(std::abs(result4.y - 1.0) < EPSILON);
    assert(std::abs(result4.z - 7.0) < EPSILON);
    
    // Test 5: At boundary
    vec3 pos5(10.0, 0.0, 5.0);
    vec3 result5 = applyPeriodicBoundary(pos5, boxSize);
    assert(std::abs(result5.x - 0.0) < EPSILON);
    assert(std::abs(result5.y - 0.0) < EPSILON);
    assert(std::abs(result5.z - 5.0) < EPSILON);
    
    std::cout << "All tests passed for applyPeriodicBoundary!" << std::endl;
    return 0;
}
