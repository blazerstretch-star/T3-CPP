#include "../src/sph_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    const float EPSILON = 0.001f;
    
    // Test 1: Basic velocity integration
    Particle p;
    p.velocity = glm::vec3(1, 0, 0);
    p.force = glm::vec3(100, 0, 0);
    p.density = 1000.0f;
    float deltaTime = 0.01f;
    float g = -9.8f;
    
    glm::vec3 oldVel = p.velocity;
    glm::vec3 expected_accel = p.force / p.density + glm::vec3(0, g, 0);
    glm::vec3 expected_vel = oldVel + expected_accel * deltaTime;
    
    integrateVelocity(&p, deltaTime, g);
    
    assert(std::abs(p.velocity.x - expected_vel.x) < EPSILON && "Velocity X mismatch");
    assert(std::abs(p.velocity.y - expected_vel.y) < EPSILON && "Velocity Y mismatch");
    assert(std::abs(p.velocity.z - expected_vel.z) < EPSILON && "Velocity Z mismatch");
    
    // Test 2: Gravity effect verification
    Particle p2;
    p2.velocity = glm::vec3(0, 0, 0);
    p2.force = glm::vec3(0, 0, 0);
    p2.density = 1000.0f;
    integrateVelocity(&p2, 0.01f, -9.8f);
    assert(p2.velocity.y < 0 && "Gravity must pull downward");
    assert(std::abs(p2.velocity.y - (-9.8f * 0.01f)) < EPSILON && "Gravity acceleration mismatch");
    
    // Test 3: Force acceleration
    Particle p3;
    p3.velocity = glm::vec3(0, 0, 0);
    p3.force = glm::vec3(1000, 0, 0);
    p3.density = 1000.0f;
    integrateVelocity(&p3, 0.01f, 0);
    float expected_vx = (1000.0f / 1000.0f) * 0.01f;
    assert(std::abs(p3.velocity.x - expected_vx) < EPSILON && "Force acceleration mismatch");
    
    // Test 4: Combined force and gravity
    Particle p4;
    p4.velocity = glm::vec3(5, 10, 0);
    p4.force = glm::vec3(500, 1000, 0);
    p4.density = 500.0f;
    glm::vec3 old_v4 = p4.velocity;
    integrateVelocity(&p4, 0.02f, -9.8f);
    glm::vec3 accel4 = p4.force / p4.density + glm::vec3(0, -9.8f, 0);
    glm::vec3 exp_v4 = old_v4 + accel4 * 0.02f;
    assert(std::abs(p4.velocity.x - exp_v4.x) < EPSILON && "Combined X velocity mismatch");
    assert(std::abs(p4.velocity.y - exp_v4.y) < EPSILON && "Combined Y velocity mismatch");
    
    // Test 5: Different time steps
    Particle p5;
    p5.velocity = glm::vec3(0, 0, 0);
    p5.force = glm::vec3(0, 0, 0);
    p5.density = 1000.0f;
    integrateVelocity(&p5, 0.1f, -9.8f);
    assert(std::abs(p5.velocity.y - (-9.8f * 0.1f)) < EPSILON && "Large timestep mismatch");
    
    // Test 6: Zero density protection (should not crash)
    Particle p6;
    p6.velocity = glm::vec3(0, 0, 0);
    p6.force = glm::vec3(100, 0, 0);
    p6.density = 0.0001f;
    integrateVelocity(&p6, 0.01f, -9.8f);
    assert(p6.velocity.x > 0 && "High acceleration from low density");

    // Test 7: Gravity must only affect Y axis, not X or Z
    Particle p7;
    p7.velocity = glm::vec3(0, 0, 0);
    p7.force = glm::vec3(0, 0, 0);
    p7.density = 1000.0f;
    integrateVelocity(&p7, 0.01f, -9.8f);
    assert(std::abs(p7.velocity.x) < EPSILON && "Gravity must not affect X velocity");
    assert(std::abs(p7.velocity.z) < EPSILON && "Gravity must not affect Z velocity");
    assert(p7.velocity.y < 0                 && "Gravity must pull Y downward");

    std::cout << "All integrateVelocity tests passed!" << std::endl;
    return 0;
}
