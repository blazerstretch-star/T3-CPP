#include "../src/md_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    const double EPSILON = 1e-6;
    
    // Test 1: Two particles at r = 2^(1/6) * sigma (minimum energy)
    double r_min = std::pow(2.0, 1.0/6.0);
    std::vector<vec3> pos1 = {vec3(0.0, 0.0, 0.0), vec3(r_min, 0.0, 0.0)};
    double pe1 = calculatePotentialEnergy(pos1, 10.0, 3.0, 1.0, 1.0);
    assert(std::abs(pe1 - (-1.0)) < EPSILON); // Minimum is -epsilon
    
    // Test 2: Two particles at r = sigma (PE = 0)
    std::vector<vec3> pos2 = {vec3(0.0, 0.0, 0.0), vec3(1.0, 0.0, 0.0)};
    double pe2 = calculatePotentialEnergy(pos2, 10.0, 3.0, 1.0, 1.0);
    assert(std::abs(pe2 - 0.0) < EPSILON);
    
    // Test 3: Two particles beyond cutoff (no interaction)
    std::vector<vec3> pos3 = {vec3(0.0, 0.0, 0.0), vec3(5.0, 0.0, 0.0)};
    double pe3 = calculatePotentialEnergy(pos3, 10.0, 2.5, 1.0, 1.0);
    assert(std::abs(pe3 - 0.0) < EPSILON);
    
    // Test 4: Three particles in line
    std::vector<vec3> pos4 = {vec3(0.0, 0.0, 0.0), vec3(1.0, 0.0, 0.0), vec3(2.0, 0.0, 0.0)};
    double pe4 = calculatePotentialEnergy(pos4, 10.0, 3.0, 1.0, 1.0);
    // PE = V(0-1) + V(0-2) + V(1-2) = 0 + V(2.0) + 0
    double expected4 = 4.0 * (std::pow(0.5, 12) - std::pow(0.5, 6));
    assert(std::abs(pe4 - expected4) < EPSILON);
    
    // Test 5: Periodic boundary - particles across boundary
    std::vector<vec3> pos5 = {vec3(0.5, 0.0, 0.0), vec3(9.5, 0.0, 0.0)};
    double pe5 = calculatePotentialEnergy(pos5, 10.0, 3.0, 1.0, 1.0);
    // Minimum image distance = 1.0, so PE = 0
    assert(std::abs(pe5 - 0.0) < EPSILON);
    
    std::cout << "All tests passed for calculatePotentialEnergy!" << std::endl;
    return 0;
}
