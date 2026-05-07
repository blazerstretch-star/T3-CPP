#include "../src/md_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    const double EPSILON = 1e-10;
    
    // Test 1: Basic velocity update
    vec3 vel1(1.0, 0.0, 0.0);
    vec3 force1(2.0, 0.0, 0.0);
    double mass1 = 1.0;
    double dt1 = 0.1;
    
    velocityVerletStep2(vel1, force1, mass1, dt1);
    
    // Expected: vel += force * (0.5 * dt / mass) = 1.0 + 2.0 * 0.05 = 1.1
    assert(std::abs(vel1.x - 1.1) < EPSILON);
    assert(std::abs(vel1.y - 0.0) < EPSILON);
    assert(std::abs(vel1.z - 0.0) < EPSILON);
    
    // Test 2: Zero force
    vec3 vel2(2.0, 3.0, 4.0);
    vec3 force2(0.0, 0.0, 0.0);
    double mass2 = 2.0;
    double dt2 = 0.2;
    
    velocityVerletStep2(vel2, force2, mass2, dt2);
    
    // Expected: vel unchanged
    assert(std::abs(vel2.x - 2.0) < EPSILON);
    assert(std::abs(vel2.y - 3.0) < EPSILON);
    assert(std::abs(vel2.z - 4.0) < EPSILON);
    
    // Test 3: 3D velocity update
    vec3 vel3(0.5, 1.0, 1.5);
    vec3 force3(1.0, 2.0, 3.0);
    double mass3 = 2.0;
    double dt3 = 0.1;
    
    velocityVerletStep2(vel3, force3, mass3, dt3);
    
    // Expected: vel += force * (0.5 * 0.1 / 2.0) = vel + force * 0.025
    assert(std::abs(vel3.x - 0.525) < EPSILON);
    assert(std::abs(vel3.y - 1.05) < EPSILON);
    assert(std::abs(vel3.z - 1.575) < EPSILON);
    
    // Test 4: Negative force (deceleration)
    vec3 vel4(5.0, 0.0, 0.0);
    vec3 force4(-10.0, 0.0, 0.0);
    double mass4 = 1.0;
    double dt4 = 0.1;
    
    velocityVerletStep2(vel4, force4, mass4, dt4);
    
    // Expected: vel += -10.0 * 0.05 = 5.0 - 0.5 = 4.5
    assert(std::abs(vel4.x - 4.5) < EPSILON);
    
    std::cout << "All tests passed for velocityVerletStep2!" << std::endl;
    return 0;
}
