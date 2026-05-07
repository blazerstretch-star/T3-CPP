#include "../src/sph_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    const float EPSILON = 0.0001f;
    float h = 0.5f;
    float elasticity = 0.5f;
    
    // Test 1: Particle below ground - position reflected above h, velocity reversed and damped
    Particle p1;
    p1.position = glm::vec3(0, 0.3f, 0);
    p1.velocity = glm::vec3(0, -1, 0);
    handleGroundCollision(&p1, h, elasticity);
    assert(p1.position.y >= h && "Reflected position must be at or above ground level h");
    assert(p1.velocity.y > 0 && "Velocity must reverse after ground collision");
    assert(std::abs(p1.velocity.y - 0.5f) < EPSILON && "Velocity damping: |-(-1)| * 0.5 = 0.5");
    
    // Test 2: Particle above ground - no change
    Particle p2;
    p2.position = glm::vec3(0, 1.0f, 0);
    p2.velocity = glm::vec3(0, -1, 0);
    glm::vec3 old_pos = p2.position;
    glm::vec3 old_vel = p2.velocity;
    handleGroundCollision(&p2, h, elasticity);
    assert(p2.position == old_pos && "Above ground position must not change");
    assert(p2.velocity == old_vel && "Above ground velocity must not change");
    
    // Test 3: Particle at exact ground level
    Particle p3;
    p3.position = glm::vec3(0, h, 0);
    p3.velocity = glm::vec3(0, -1, 0);
    glm::vec3 pos3 = p3.position;
    handleGroundCollision(&p3, h, elasticity);
    assert(p3.position == pos3 && "At ground level must not change");
    
    // Test 4: Different elasticity values
    Particle p4;
    p4.position = glm::vec3(0, 0.3f, 0);
    p4.velocity = glm::vec3(0, -2, 0);
    handleGroundCollision(&p4, h, 0.8f);
    assert(std::abs(p4.velocity.y - 1.6f) < EPSILON && "High elasticity damping mismatch");
    
    Particle p5;
    p5.position = glm::vec3(0, 0.3f, 0);
    p5.velocity = glm::vec3(0, -2, 0);
    handleGroundCollision(&p5, h, 0.2f);
    assert(std::abs(p5.velocity.y - 0.4f) < EPSILON && "Low elasticity damping mismatch");
    
    // Test 5: Horizontal velocity unchanged
    Particle p6;
    p6.position = glm::vec3(5, 0.3f, 10);
    p6.velocity = glm::vec3(3, -1, 7);
    handleGroundCollision(&p6, h, elasticity);
    assert(std::abs(p6.velocity.x - 3.0f) < EPSILON && "X velocity must not change");
    assert(std::abs(p6.velocity.z - 7.0f) < EPSILON && "Z velocity must not change");
    
    // Test 6: Very close to ground
    Particle p7;
    p7.position = glm::vec3(0, 0.49f, 0);
    p7.velocity = glm::vec3(0, -1, 0);
    handleGroundCollision(&p7, h, elasticity);
    assert(p7.position.y >= h && "Must be above ground after collision");
    assert(p7.velocity.y > 0 && "Velocity must reverse");
    
    // Test 7: Zero elasticity (perfectly inelastic)
    Particle p8;
    p8.position = glm::vec3(0, 0.3f, 0);
    p8.velocity = glm::vec3(0, -1, 0);
    handleGroundCollision(&p8, h, 0.0f);
    assert(std::abs(p8.velocity.y) < EPSILON && "Zero elasticity must stop particle");
    
    std::cout << "All handleGroundCollision tests passed!" << std::endl;
    return 0;
}
