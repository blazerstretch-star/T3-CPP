#include "../src/md_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    const double EPSILON = 1e-10;
    
    // Test 1: Basic integration with constant force
    vec3 pos1(0.0, 0.0, 0.0);
    vec3 vel1(1.0, 0.0, 0.0);
    vec3 force1(2.0, 0.0, 0.0);
    double mass1 = 1.0;
    double dt1 = 0.1;
    
    velocityVerletStep1(pos1, vel1, force1, mass1, dt1);
    
    // Expected: vel += force * (0.5 * dt / mass) = 1.0 + 2.0 * 0.05 = 1.1
    // Expected: pos += vel * dt = 0.0 + 1.1 * 0.1 = 0.11
    assert(std::abs(vel1.x - 1.1) < EPSILON);
    assert(std::abs(pos1.x - 0.11) < EPSILON);
    
    // Test 2: Zero force
    vec3 pos2(5.0, 5.0, 5.0);
    vec3 vel2(2.0, 3.0, 4.0);
    vec3 force2(0.0, 0.0, 0.0);
    double mass2 = 2.0;
    double dt2 = 0.2;
    
    velocityVerletStep1(pos2, vel2, force2, mass2, dt2);
    
    // Expected: vel unchanged = (2.0, 3.0, 4.0)
    // Expected: pos += vel * dt = (5.4, 5.6, 5.8)
    assert(std::abs(vel2.x - 2.0) < EPSILON);
    assert(std::abs(vel2.y - 3.0) < EPSILON);
    assert(std::abs(vel2.z - 4.0) < EPSILON);
    assert(std::abs(pos2.x - 5.4) < EPSILON);
    assert(std::abs(pos2.y - 5.6) < EPSILON);
    assert(std::abs(pos2.z - 5.8) < EPSILON);
    
    // Test 3: 3D motion with force
    vec3 pos3(1.0, 2.0, 3.0);
    vec3 vel3(0.5, 1.0, 1.5);
    vec3 force3(1.0, 2.0, 3.0);
    double mass3 = 2.0;
    double dt3 = 0.1;
    
    velocityVerletStep1(pos3, vel3, force3, mass3, dt3);
    
    // Expected: vel += force * (0.5 * 0.1 / 2.0) = vel + force * 0.025
    // vel = (0.525, 1.05, 1.575)
    // pos += vel * dt = (1.0525, 2.105, 3.1575)
    assert(std::abs(vel3.x - 0.525) < EPSILON);
    assert(std::abs(vel3.y - 1.05) < EPSILON);
    assert(std::abs(vel3.z - 1.575) < EPSILON);
    assert(std::abs(pos3.x - 1.0525) < EPSILON);
    assert(std::abs(pos3.y - 2.105) < EPSILON);
    assert(std::abs(pos3.z - 3.1575) < EPSILON);
    
    std::cout << "All tests passed for velocityVerletStep1!" << std::endl;
    return 0;
}
