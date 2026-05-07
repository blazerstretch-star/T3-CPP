#include "../src/sph_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    const float EPSILON = 0.001f;
    
    // Test 1: Particles within smoothing radius
    Particle pi, pj;
    pi.position = glm::vec3(0, 0, 0);
    pi.pressure = 1000.0f;
    pi.density = 1000.0f;
    pj.position = glm::vec3(0.1f, 0, 0);
    pj.pressure = 1100.0f;
    pj.density = 1000.0f;
    
    float mass = 1.0f;
    float spikyGrad = -100.0f;
    float h = 0.5f;
    
    glm::vec3 force = calculatePressureForce(&pi, &pj, mass, spikyGrad, h);
    assert(glm::length(force) > 0 && "Force must be non-zero for nearby particles");
    
    // Test 2: Verify force direction (should point from high to low pressure)
    // Recompute expected force without glm::normalize to avoid NaN on zero-length vectors
    float dist = glm::length(pj.position - pi.position);
    glm::vec3 diff = pj.position - pi.position;
    glm::vec3 dir = diff / dist;  // safe: positions are distinct
    float pressure_term = (pi.pressure + pj.pressure) / (2 * pj.density);
    float kernel_term = std::pow(h - dist, 2);
    glm::vec3 expected_force = -dir * mass * pressure_term * spikyGrad * kernel_term;
    assert(std::abs(force.x - expected_force.x) < EPSILON && "Force X component mismatch");
    assert(std::abs(force.y - expected_force.y) < EPSILON && "Force Y component mismatch");
    assert(std::abs(force.z - expected_force.z) < EPSILON && "Force Z component mismatch");
    
    // Test 3: Particles outside smoothing radius produce zero force
    pj.position = glm::vec3(10.0f, 0, 0);
    glm::vec3 force_far = calculatePressureForce(&pi, &pj, mass, spikyGrad, h);
    assert(glm::length(force_far) == 0 && "Force must be zero outside smoothing radius");
    assert(force_far.x == 0 && force_far.y == 0 && force_far.z == 0 && "All components must be zero");
    
    // Test 4: Particles at exact smoothing radius boundary
    pj.position = glm::vec3(h, 0, 0);
    glm::vec3 force_boundary = calculatePressureForce(&pi, &pj, mass, spikyGrad, h);
    assert(glm::length(force_boundary) == 0 && "Force at boundary must be zero");
    
    // Test 5: Equal pressure particles
    pj.position = glm::vec3(0.1f, 0, 0);
    pi.pressure = 1000.0f;
    pj.pressure = 1000.0f;
    glm::vec3 force_equal = calculatePressureForce(&pi, &pj, mass, spikyGrad, h);
    assert(glm::length(force_equal) > 0 && "Force exists even with equal pressure");
    
    // Test 6: Different axis alignment
    pj.position = glm::vec3(0, 0.2f, 0);
    glm::vec3 force_y = calculatePressureForce(&pi, &pj, mass, spikyGrad, h);
    assert(force_y.y != 0 && "Y component must be non-zero");
    assert(std::abs(force_y.x) < EPSILON && "X component must be near zero");
    assert(std::abs(force_y.z) < EPSILON && "Z component must be near zero");
    
    // Test 7: Force magnitude decreases with distance
    pj.position = glm::vec3(0.1f, 0, 0);
    glm::vec3 force_close = calculatePressureForce(&pi, &pj, mass, spikyGrad, h);
    pj.position = glm::vec3(0.3f, 0, 0);
    glm::vec3 force_medium = calculatePressureForce(&pi, &pj, mass, spikyGrad, h);
    assert(glm::length(force_close) > glm::length(force_medium) && "Closer particles produce stronger force");

    // Test 8: Asymmetric densities — exposes whether pj.density is used (not pi.density or average)
    // formula: -(pi.pressure + pj.pressure) / (2 * pj.density)
    pi.density = 800.0f; pj.density = 1200.0f;
    pi.pressure = 1000.0f; pj.pressure = 1000.0f;
    pj.position = glm::vec3(0.1f, 0, 0);
    {
        float dist = 0.1f;
        float expected_x = -(1.0f) * mass * ((1000.0f + 1000.0f) / (2.0f * 1200.0f)) * spikyGrad * std::pow(h - dist, 2);
        glm::vec3 f = calculatePressureForce(&pi, &pj, mass, spikyGrad, h);
        assert(std::abs(f.x - expected_x) < EPSILON && "Asymmetric density: must use pj.density");
    }

    std::cout << "All calculatePressureForce tests passed!" << std::endl;
    return 0;
}
