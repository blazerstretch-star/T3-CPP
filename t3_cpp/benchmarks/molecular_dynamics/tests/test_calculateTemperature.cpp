#include "../src/md_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    const double EPSILON = 1e-10;
    const double kB = 1.0; // Boltzmann constant in reduced units
    
    // Test 1: Single particle at rest
    std::vector<vec3> vel1 = {vec3(0.0, 0.0, 0.0)};
    std::vector<double> mass1 = {1.0};
    double temp1 = calculateTemperature(vel1, mass1);
    assert(std::abs(temp1 - 0.0) < EPSILON);
    
    // Test 2: Single particle with known kinetic energy
    // T = 2*KE / (3*N*kB) = 2*KE / 3 (for N=1, kB=1)
    std::vector<vec3> vel2 = {vec3(3.0, 0.0, 0.0)};
    std::vector<double> mass2 = {1.0};
    double temp2 = calculateTemperature(vel2, mass2);
    // KE = 0.5 * 1.0 * 9.0 = 4.5
    // T = 2 * 4.5 / 3 = 3.0
    assert(std::abs(temp2 - 3.0) < EPSILON);
    
    // Test 3: Multiple particles
    std::vector<vec3> vel3 = {
        vec3(1.0, 0.0, 0.0),
        vec3(0.0, 1.0, 0.0)
    };
    std::vector<double> mass3 = {1.0, 1.0};
    double temp3 = calculateTemperature(vel3, mass3);
    // KE = 0.5 * 1.0 * 1.0 + 0.5 * 1.0 * 1.0 = 1.0
    // T = 2 * 1.0 / (3 * 2) = 2.0 / 6.0 = 1/3
    assert(std::abs(temp3 - (1.0/3.0)) < EPSILON);
    
    // Test 4: 3D velocities
    std::vector<vec3> vel4 = {
        vec3(1.0, 1.0, 1.0),
        vec3(1.0, 1.0, 1.0),
        vec3(1.0, 1.0, 1.0)
    };
    std::vector<double> mass4 = {1.0, 1.0, 1.0};
    double temp4 = calculateTemperature(vel4, mass4);
    // KE = 3 * 0.5 * 1.0 * 3.0 = 4.5
    // T = 2 * 4.5 / (3 * 3) = 9.0 / 9.0 = 1.0
    assert(std::abs(temp4 - 1.0) < EPSILON);
    
    // Test 5: Different masses
    std::vector<vec3> vel5 = {
        vec3(2.0, 0.0, 0.0),
        vec3(1.0, 0.0, 0.0)
    };
    std::vector<double> mass5 = {2.0, 4.0};
    double temp5 = calculateTemperature(vel5, mass5);
    // KE = 0.5 * 2.0 * 4.0 + 0.5 * 4.0 * 1.0 = 4.0 + 2.0 = 6.0
    // T = 2 * 6.0 / (3 * 2) = 12.0 / 6.0 = 2.0
    assert(std::abs(temp5 - 2.0) < EPSILON);
    
    std::cout << "All tests passed for calculateTemperature!" << std::endl;
    return 0;
}
