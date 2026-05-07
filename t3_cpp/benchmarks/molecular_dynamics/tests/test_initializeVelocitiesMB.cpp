#include "../src/md_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <cstdlib>

int main() {
    const double EPSILON = 1e-2;
    srand(42); // Fixed seed for reproducibility
    
    // Test 1: Center of mass velocity should be zero
    std::vector<vec3> vel1(10);
    std::vector<double> mass1(10, 1.0);
    initializeVelocitiesMB(vel1, mass1, 1.0);
    
    vec3 vcm(0, 0, 0);
    for (size_t i = 0; i < vel1.size(); ++i) {
        vcm += vel1[i] * mass1[i];
    }
    assert(std::abs(vcm.x) < EPSILON);
    assert(std::abs(vcm.y) < EPSILON);
    assert(std::abs(vcm.z) < EPSILON);
    
    // Test 2: Average kinetic energy should match temperature
    std::vector<vec3> vel2(100);
    std::vector<double> mass2(100, 1.0);
    initializeVelocitiesMB(vel2, mass2, 2.0);
    
    double ke = 0.0;
    for (size_t i = 0; i < vel2.size(); ++i) {
        ke += 0.5 * mass2[i] * vel2[i].lengthSquared();
    }
    double temp = (2.0 * ke) / (3.0 * vel2.size());
    assert(std::abs(temp - 2.0) < 0.3); // Within 15%
    
    // Test 3: Different masses should have different velocity distributions
    std::vector<vec3> vel3(2);
    std::vector<double> mass3 = {1.0, 4.0};
    initializeVelocitiesMB(vel3, mass3, 1.0);
    
    // Lighter particle should have higher average speed
    // (averaged over many initializations, but check they're different)
    assert(vel3[0].lengthSquared() != vel3[1].lengthSquared());
    
    // Test 4: Higher temperature should give higher velocities
    std::vector<vec3> vel4a(50);
    std::vector<double> mass4(50, 1.0);
    initializeVelocitiesMB(vel4a, mass4, 1.0);
    
    std::vector<vec3> vel4b(50);
    initializeVelocitiesMB(vel4b, mass4, 4.0);
    
    double ke_low = 0.0, ke_high = 0.0;
    for (size_t i = 0; i < 50; ++i) {
        ke_low += 0.5 * vel4a[i].lengthSquared();
        ke_high += 0.5 * vel4b[i].lengthSquared();
    }
    assert(ke_high > ke_low);
    
    // Test 5: Velocities should be non-zero
    std::vector<vec3> vel5(10);
    std::vector<double> mass5(10, 1.0);
    initializeVelocitiesMB(vel5, mass5, 1.0);
    
    bool has_nonzero = false;
    for (const auto& v : vel5) {
        if (v.lengthSquared() > EPSILON) {
            has_nonzero = true;
            break;
        }
    }
    assert(has_nonzero);
    
    std::cout << "All tests passed for initializeVelocitiesMB!" << std::endl;
    return 0;
}
