#include "../src/sph_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    const float EPSILON = 0.00001f;
    
    // Test 1: Basic position integration
    Particle p;
    p.position = glm::vec3(0, 0, 0);
    p.velocity = glm::vec3(1, 2, 3);
    float deltaTime = 0.1f;
    
    integratePosition(&p, deltaTime);
    assert(std::abs(p.position.x - 0.1f) < EPSILON && "Position X mismatch");
    assert(std::abs(p.position.y - 0.2f) < EPSILON && "Position Y mismatch");
    assert(std::abs(p.position.z - 0.3f) < EPSILON && "Position Z mismatch");
    
    // Test 2: Different time step
    Particle p2;
    p2.position = glm::vec3(5, 10, 15);
    p2.velocity = glm::vec3(2, 4, 6);
    integratePosition(&p2, 0.5f);
    assert(std::abs(p2.position.x - 6.0f) < EPSILON && "Large timestep X mismatch");
    assert(std::abs(p2.position.y - 12.0f) < EPSILON && "Large timestep Y mismatch");
    assert(std::abs(p2.position.z - 18.0f) < EPSILON && "Large timestep Z mismatch");
    
    // Test 3: Negative velocity
    Particle p3;
    p3.position = glm::vec3(10, 10, 10);
    p3.velocity = glm::vec3(-5, -10, -15);
    integratePosition(&p3, 0.1f);
    assert(std::abs(p3.position.x - 9.5f) < EPSILON && "Negative velocity X mismatch");
    assert(std::abs(p3.position.y - 9.0f) < EPSILON && "Negative velocity Y mismatch");
    assert(std::abs(p3.position.z - 8.5f) < EPSILON && "Negative velocity Z mismatch");
    
    // Test 4: Zero velocity
    Particle p4;
    p4.position = glm::vec3(1, 2, 3);
    p4.velocity = glm::vec3(0, 0, 0);
    glm::vec3 old_pos = p4.position;
    integratePosition(&p4, 0.1f);
    assert(p4.position == old_pos && "Zero velocity must not change position");
    
    // Test 5: Small time step
    Particle p5;
    p5.position = glm::vec3(0, 0, 0);
    p5.velocity = glm::vec3(100, 100, 100);
    integratePosition(&p5, 0.001f);
    assert(std::abs(p5.position.x - 0.1f) < EPSILON && "Small timestep X mismatch");
    assert(std::abs(p5.position.y - 0.1f) < EPSILON && "Small timestep Y mismatch");
    assert(std::abs(p5.position.z - 0.1f) < EPSILON && "Small timestep Z mismatch");
    
    // Test 6: Multiple integrations
    Particle p6;
    p6.position = glm::vec3(0, 0, 0);
    p6.velocity = glm::vec3(1, 1, 1);
    integratePosition(&p6, 0.1f);
    integratePosition(&p6, 0.1f);
    assert(std::abs(p6.position.x - 0.2f) < EPSILON && "Multiple integration X mismatch");
    assert(std::abs(p6.position.y - 0.2f) < EPSILON && "Multiple integration Y mismatch");
    assert(std::abs(p6.position.z - 0.2f) < EPSILON && "Multiple integration Z mismatch");
    
    // Test 7: Mixed positive/negative velocity
    Particle p7;
    p7.position = glm::vec3(5, 5, 5);
    p7.velocity = glm::vec3(10, -10, 0);
    integratePosition(&p7, 0.1f);
    assert(std::abs(p7.position.x - 6.0f) < EPSILON && "Mixed velocity X mismatch");
    assert(std::abs(p7.position.y - 4.0f) < EPSILON && "Mixed velocity Y mismatch");
    assert(std::abs(p7.position.z - 5.0f) < EPSILON && "Mixed velocity Z mismatch");
    
    std::cout << "All integratePosition tests passed!" << std::endl;
    return 0;
}
