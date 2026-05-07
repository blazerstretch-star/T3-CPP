#include "../src/md_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    const double EPSILON = 1e-10;
    
    // Test 1: Single particle at rest
    std::vector<vec3> vel1 = {vec3(0.0, 0.0, 0.0)};
    std::vector<double> mass1 = {1.0};
    double ke1 = calculateKineticEnergy(vel1, mass1);
    assert(std::abs(ke1 - 0.0) < EPSILON);
    
    // Test 2: Single particle moving
    std::vector<vec3> vel2 = {vec3(2.0, 0.0, 0.0)};
    std::vector<double> mass2 = {1.0};
    double ke2 = calculateKineticEnergy(vel2, mass2);
    // KE = 0.5 * 1.0 * 4.0 = 2.0
    assert(std::abs(ke2 - 2.0) < EPSILON);
    
    // Test 3: Multiple particles
    std::vector<vec3> vel3 = {
        vec3(1.0, 0.0, 0.0),
        vec3(0.0, 2.0, 0.0),
        vec3(0.0, 0.0, 3.0)
    };
    std::vector<double> mass3 = {1.0, 1.0, 1.0};
    double ke3 = calculateKineticEnergy(vel3, mass3);
    // KE = 0.5 * (1 + 4 + 9) = 7.0
    assert(std::abs(ke3 - 7.0) < EPSILON);
    
    // Test 4: Different masses
    std::vector<vec3> vel4 = {
        vec3(2.0, 0.0, 0.0),
        vec3(1.0, 0.0, 0.0)
    };
    std::vector<double> mass4 = {2.0, 4.0};
    double ke4 = calculateKineticEnergy(vel4, mass4);
    // KE = 0.5 * 2.0 * 4.0 + 0.5 * 4.0 * 1.0 = 4.0 + 2.0 = 6.0
    assert(std::abs(ke4 - 6.0) < EPSILON);
    
    // Test 5: 3D velocities
    std::vector<vec3> vel5 = {
        vec3(1.0, 1.0, 1.0),
        vec3(2.0, 2.0, 2.0)
    };
    std::vector<double> mass5 = {1.0, 1.0};
    double ke5 = calculateKineticEnergy(vel5, mass5);
    // KE = 0.5 * 1.0 * 3.0 + 0.5 * 1.0 * 12.0 = 1.5 + 6.0 = 7.5
    assert(std::abs(ke5 - 7.5) < EPSILON);
    
    std::cout << "All tests passed for calculateKineticEnergy!" << std::endl;
    return 0;
}
