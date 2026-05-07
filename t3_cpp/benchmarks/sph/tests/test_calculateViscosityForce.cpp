#include "../src/sph_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    const float EPSILON = 0.001f;
    
    // Test 1: Basic viscosity force calculation
    Particle pi, pj;
    pi.position = glm::vec3(0, 0, 0);
    pi.velocity = glm::vec3(1, 0, 0);
    pi.density = 1000.0f;
    pj.position = glm::vec3(0.1f, 0, 0);
    pj.velocity = glm::vec3(2, 0, 0);
    pj.density = 1000.0f;
    
    float mass = 1.0f;
    float viscosity = 0.01f;
    float spikyLap = 100.0f;
    float h = 0.5f;
    
    glm::vec3 force = calculateViscosityForce(&pi, &pj, mass, viscosity, spikyLap, h);
    assert(glm::length(force) > 0 && "Force must be non-zero for different velocities");
    
    // Test 2: Verify force direction (toward higher velocity)
    float dist = glm::length(pj.position - pi.position);
    glm::vec3 velocityDif = pj.velocity - pi.velocity;
    glm::vec3 expected_force = viscosity * mass * (velocityDif / pj.density) * spikyLap * (h - dist);
    assert(std::abs(force.x - expected_force.x) < EPSILON && "Force X mismatch");
    assert(force.x > 0 && "Force must point toward higher velocity");
    
    // Test 3: Particles outside smoothing radius
    pj.position = glm::vec3(10.0f, 0, 0);
    glm::vec3 force_far = calculateViscosityForce(&pi, &pj, mass, viscosity, spikyLap, h);
    assert(glm::length(force_far) == 0 && "Outside radius must give zero force");
    
    // Test 4: Same velocity gives zero force
    pj.position = glm::vec3(0.1f, 0, 0);
    pi.velocity = glm::vec3(5, 5, 5);
    pj.velocity = glm::vec3(5, 5, 5);
    glm::vec3 force_same = calculateViscosityForce(&pi, &pj, mass, viscosity, spikyLap, h);
    assert(glm::length(force_same) < EPSILON && "Same velocity must give zero force");
    
    // Test 5: Opposite velocities
    pi.velocity = glm::vec3(1, 0, 0);
    pj.velocity = glm::vec3(-1, 0, 0);
    glm::vec3 force_opposite = calculateViscosityForce(&pi, &pj, mass, viscosity, spikyLap, h);
    assert(force_opposite.x < 0 && "Force must point toward pj velocity");
    
    // Test 6: Force magnitude decreases with distance
    pj.velocity = glm::vec3(2, 0, 0);
    pj.position = glm::vec3(0.1f, 0, 0);
    glm::vec3 force_close = calculateViscosityForce(&pi, &pj, mass, viscosity, spikyLap, h);
    pj.position = glm::vec3(0.3f, 0, 0);
    glm::vec3 force_medium = calculateViscosityForce(&pi, &pj, mass, viscosity, spikyLap, h);
    assert(glm::length(force_close) > glm::length(force_medium) && "Closer particles produce stronger force");
    
    // Test 7: Viscosity coefficient scaling
    pj.position = glm::vec3(0.1f, 0, 0);
    glm::vec3 force_v1 = calculateViscosityForce(&pi, &pj, mass, 0.01f, spikyLap, h);
    glm::vec3 force_v2 = calculateViscosityForce(&pi, &pj, mass, 0.02f, spikyLap, h);
    assert(std::abs(glm::length(force_v2) / glm::length(force_v1) - 2.0f) < 0.01f && "Viscosity scaling violated");

    // Test 8: Asymmetric densities — exposes whether pj.density is used (not pi.density or average)
    // formula: viscosity * mass * (velDiff / pj.density) * spikyLap * (h - dist)
    pi.density = 500.0f; pj.density = 2000.0f;
    pi.velocity = glm::vec3(0, 0, 0); pj.velocity = glm::vec3(4, 0, 0);
    pj.position = glm::vec3(0.1f, 0, 0);
    {
        float dist = 0.1f;
        float expected_x = viscosity * mass * (4.0f / 2000.0f) * spikyLap * (h - dist);
        glm::vec3 f = calculateViscosityForce(&pi, &pj, mass, viscosity, spikyLap, h);
        assert(std::abs(f.x - expected_x) < EPSILON && "Asymmetric density: must use pj.density");
    }

    std::cout << "All calculateViscosityForce tests passed!" << std::endl;
    return 0;
}
