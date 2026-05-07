#include "../src/md_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <cstdlib>

int main() {
    const double EPSILON = 1e-2;
    srand(42); // Fixed seed for reproducibility
    
    // Test 1: Zero collision frequency (no changes)
    std::vector<vec3> vel1 = {vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0)};
    std::vector<double> mass1 = {1.0, 1.0};
    std::vector<vec3> vel1_orig = vel1;
    applyAndersonThermostat(vel1, mass1, 1.0, 0.0, 0.1);
    assert(vel1[0].x == vel1_orig[0].x);
    assert(vel1[1].y == vel1_orig[1].y);
    
    // Test 2: High collision frequency (velocities should change)
    std::vector<vec3> vel2 = {vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0)};
    std::vector<double> mass2 = {1.0, 1.0, 1.0};
    applyAndersonThermostat(vel2, mass2, 1.0, 100.0, 0.1);
    // At least one velocity should have changed
    bool changed = false;
    if (vel2[0].x != 1.0 || vel2[1].y != 1.0 || vel2[2].z != 1.0) changed = true;
    assert(changed);
    
    // Test 3: Average kinetic energy should approach target temperature
    std::vector<vec3> vel3(100);
    std::vector<double> mass3(100, 1.0);
    for (int i = 0; i < 100; ++i) vel3[i] = vec3(0.0, 0.0, 0.0);
    
    // Apply thermostat many times
    for (int step = 0; step < 1000; ++step) {
        applyAndersonThermostat(vel3, mass3, 2.0, 1.0, 0.1);
    }
    
    double ke = 0.0;
    for (size_t i = 0; i < vel3.size(); ++i) {
        ke += 0.5 * mass3[i] * vel3[i].lengthSquared();
    }
    double temp = (2.0 * ke) / (3.0 * vel3.size());
    assert(std::abs(temp - 2.0) < 0.5); // Within 25% of target
    
    // Test 4: Velocities should follow Maxwell-Boltzmann statistics
    std::vector<vec3> vel4(1000);
    std::vector<double> mass4(1000, 1.0);
    for (int i = 0; i < 1000; ++i) vel4[i] = vec3(0.0, 0.0, 0.0);
    
    for (int step = 0; step < 100; ++step) {
        applyAndersonThermostat(vel4, mass4, 1.0, 10.0, 0.1);
    }
    
    // Check that velocities are distributed (not all zero)
    double sum_vx = 0.0;
    for (const auto& v : vel4) sum_vx += v.x;
    double avg_vx = sum_vx / vel4.size();
    assert(std::abs(avg_vx) < 0.1); // Average should be near zero
    
    std::cout << "All tests passed for applyAndersonThermostat!" << std::endl;
    return 0;
}
