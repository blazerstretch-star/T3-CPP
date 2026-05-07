#include "../src/sph_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    const float EPSILON = 0.01f;
    float h = 0.5f;
    float boxWidth = 8.0f;
    float elasticity = 0.5f;
    
    // Test 1: Left wall collision
    Particle p1;
    p1.position = glm::vec3(-8.0f, 1, 1);
    p1.velocity = glm::vec3(-1, 0, 0);
    handleWallCollisionX(&p1, h, boxWidth, elasticity);
    assert(p1.velocity.x > 0 && "Left wall velocity must reverse");
    assert(std::abs(p1.velocity.x - 0.5f) < EPSILON && "Left wall damping mismatch");
    assert(p1.position.x > h - boxWidth && "Reflected position must be inside left boundary");
    
    // Test 2: Right wall collision
    Particle p2;
    p2.position = glm::vec3(8.0f, 1, 1);
    p2.velocity = glm::vec3(1, 0, 0);
    handleWallCollisionX(&p2, h, boxWidth, elasticity);
    assert(p2.velocity.x < 0 && "Right wall velocity must reverse");
    assert(std::abs(p2.velocity.x + 0.5f) < EPSILON && "Right wall damping mismatch");
    
    // Test 3: No collision in safe zone
    Particle p3;
    p3.position = glm::vec3(0, 1, 1);
    p3.velocity = glm::vec3(1, 0, 0);
    glm::vec3 old_pos = p3.position;
    glm::vec3 old_vel = p3.velocity;
    handleWallCollisionX(&p3, h, boxWidth, elasticity);
    assert(p3.position == old_pos && "Safe zone position must not change");
    assert(p3.velocity == old_vel && "Safe zone velocity must not change");
    
    // Test 4: Different elasticity - left wall
    Particle p4;
    p4.position = glm::vec3(-8.0f, 1, 1);
    p4.velocity = glm::vec3(-2, 0, 0);
    handleWallCollisionX(&p4, h, boxWidth, 0.8f);
    assert(std::abs(p4.velocity.x - 1.6f) < EPSILON && "High elasticity left wall mismatch");
    
    // Test 5: Different elasticity - right wall
    Particle p5;
    p5.position = glm::vec3(8.0f, 1, 1);
    p5.velocity = glm::vec3(2, 0, 0);
    handleWallCollisionX(&p5, h, boxWidth, 0.3f);
    assert(std::abs(p5.velocity.x + 0.6f) < EPSILON && "Low elasticity right wall mismatch");
    
    // Test 6: Y and Z velocities unchanged
    Particle p6;
    p6.position = glm::vec3(-8.0f, 1, 1);
    p6.velocity = glm::vec3(-1, 5, 10);
    handleWallCollisionX(&p6, h, boxWidth, elasticity);
    assert(std::abs(p6.velocity.y - 5.0f) < EPSILON && "Y velocity must not change");
    assert(std::abs(p6.velocity.z - 10.0f) < EPSILON && "Z velocity must not change");
    
    // Test 7: At exact boundary
    Particle p7;
    p7.position = glm::vec3(h - boxWidth, 1, 1);
    p7.velocity = glm::vec3(-1, 0, 0);
    glm::vec3 pos7 = p7.position;
    handleWallCollisionX(&p7, h, boxWidth, elasticity);
    assert(p7.position == pos7 && "At boundary must not change");
    
    std::cout << "All handleWallCollisionX tests passed!" << std::endl;
    return 0;
}
