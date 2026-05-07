#include "../src/sph_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    const float EPSILON = 0.01f;
    
    // Test 1: Basic force accumulation
    Particle pi, pj;
    pi.position = glm::vec3(0, 0, 0);
    pi.velocity = glm::vec3(1, 0, 0);
    pi.pressure = 1000.0f;
    pi.density = 1000.0f;
    pj.position = glm::vec3(0.1f, 0, 0);
    pj.velocity = glm::vec3(2, 0, 0);
    pj.pressure = 1100.0f;
    pj.density = 1000.0f;
    
    float mass = 1.0f;
    float viscosity = 0.01f;
    float spikyGrad = -100.0f;
    float spikyLap = 100.0f;
    float h = 0.5f;
    glm::vec3 force(0, 0, 0);
    
    accumulateForcesFromNeighbor(&pi, &pj, mass, viscosity, spikyGrad, spikyLap, h, force);
    assert(glm::length(force) > 0 && "Force must be non-zero");
    
    // Test 2: Accumulation adds to existing force
    glm::vec3 force2(10, 0, 0);
    glm::vec3 old_force = force2;
    accumulateForcesFromNeighbor(&pi, &pj, mass, viscosity, spikyGrad, spikyLap, h, force2);
    assert(glm::length(force2) > glm::length(old_force) && "Must accumulate to existing force");
    
    // Test 3: Outside smoothing radius gives no contribution
    pj.position = glm::vec3(10.0f, 0, 0);
    glm::vec3 force3(0, 0, 0);
    accumulateForcesFromNeighbor(&pi, &pj, mass, viscosity, spikyGrad, spikyLap, h, force3);
    assert(glm::length(force3) == 0 && "Outside radius must contribute zero");
    
    // Test 4: Verify pressure and viscosity components combined
    pj.position = glm::vec3(0.1f, 0, 0);
    glm::vec3 force_combined(0, 0, 0);
    accumulateForcesFromNeighbor(&pi, &pj, mass, viscosity, spikyGrad, spikyLap, h, force_combined);
    
    glm::vec3 pressure_only = calculatePressureForce(&pi, &pj, mass, spikyGrad, h);
    glm::vec3 viscosity_only = calculateViscosityForce(&pi, &pj, mass, viscosity, spikyLap, h);
    glm::vec3 expected_combined = pressure_only + viscosity_only;
    
    assert(std::abs(force_combined.x - expected_combined.x) < EPSILON && "Combined force X mismatch");
    assert(std::abs(force_combined.y - expected_combined.y) < EPSILON && "Combined force Y mismatch");
    assert(std::abs(force_combined.z - expected_combined.z) < EPSILON && "Combined force Z mismatch");
    
    // Test 5: Multiple accumulations
    glm::vec3 force_multi(0, 0, 0);
    pj.position = glm::vec3(0.1f, 0, 0);
    accumulateForcesFromNeighbor(&pi, &pj, mass, viscosity, spikyGrad, spikyLap, h, force_multi);
    glm::vec3 first_contrib = force_multi;
    pj.position = glm::vec3(0, 0.1f, 0);
    accumulateForcesFromNeighbor(&pi, &pj, mass, viscosity, spikyGrad, spikyLap, h, force_multi);
    assert(glm::length(force_multi) > glm::length(first_contrib) && "Multiple accumulations must add");
    
    // Test 6: At boundary (dist = h)
    pj.position = glm::vec3(h, 0, 0);
    glm::vec3 force_boundary(0, 0, 0);
    accumulateForcesFromNeighbor(&pi, &pj, mass, viscosity, spikyGrad, spikyLap, h, force_boundary);
    assert(glm::length(force_boundary) == 0 && "At boundary must contribute zero");
    
    // Test 7: Closer neighbors contribute more
    pj.position = glm::vec3(0.05f, 0, 0);
    glm::vec3 force_close(0, 0, 0);
    accumulateForcesFromNeighbor(&pi, &pj, mass, viscosity, spikyGrad, spikyLap, h, force_close);
    pj.position = glm::vec3(0.3f, 0, 0);
    glm::vec3 force_far(0, 0, 0);
    accumulateForcesFromNeighbor(&pi, &pj, mass, viscosity, spikyGrad, spikyLap, h, force_far);
    assert(glm::length(force_close) > glm::length(force_far) && "Closer neighbors must contribute more");
    
    std::cout << "All accumulateForcesFromNeighbor tests passed!" << std::endl;
    return 0;
}
