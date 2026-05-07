#include "../src/sph_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    const float EPSILON = 0.01f;
    float h = 0.5f;
    float boxWidth = 8.0f;
    float elasticity = 0.5f;
    
    // Test 1: Front wall collision
    Particle p1;
    p1.position = glm::vec3(1, 1, -8.0f);
    p1.velocity = glm::vec3(0, 0, -1);
    handleWallCollisionZ(&p1, h, boxWidth, elasticity);
    assert(p1.velocity.z > 0 && "Front wall velocity must reverse");
    assert(std::abs(p1.velocity.z - 0.5f) < EPSILON && "Front wall damping mismatch");
    assert(p1.position.z > h - boxWidth && "Reflected position must be inside front boundary");
    
    // Test 2: Back wall collision
    Particle p2;
    p2.position = glm::vec3(1, 1, 8.0f);
    p2.velocity = glm::vec3(0, 0, 1);
    handleWallCollisionZ(&p2, h, boxWidth, elasticity);
    assert(p2.velocity.z < 0 && "Back wall velocity must reverse");
    assert(std::abs(p2.velocity.z + 0.5f) < EPSILON && "Back wall damping mismatch");
    
    // Test 3: No collision in safe zone
    Particle p3;
    p3.position = glm::vec3(1, 1, 0);
    p3.velocity = glm::vec3(0, 0, 1);
    glm::vec3 old_pos = p3.position;
    glm::vec3 old_vel = p3.velocity;
    handleWallCollisionZ(&p3, h, boxWidth, elasticity);
    assert(p3.position == old_pos && "Safe zone position must not change");
    assert(p3.velocity == old_vel && "Safe zone velocity must not change");
    
    // Test 4: Different elasticity - front wall
    Particle p4;
    p4.position = glm::vec3(1, 1, -8.0f);
    p4.velocity = glm::vec3(0, 0, -2);
    handleWallCollisionZ(&p4, h, boxWidth, 0.8f);
    assert(std::abs(p4.velocity.z - 1.6f) < EPSILON && "High elasticity front wall mismatch");
    
    // Test 5: Different elasticity - back wall
    Particle p5;
    p5.position = glm::vec3(1, 1, 8.0f);
    p5.velocity = glm::vec3(0, 0, 2);
    handleWallCollisionZ(&p5, h, boxWidth, 0.3f);
    assert(std::abs(p5.velocity.z + 0.6f) < EPSILON && "Low elasticity back wall mismatch");
    
    // Test 6: X and Y velocities unchanged
    Particle p6;
    p6.position = glm::vec3(1, 1, -8.0f);
    p6.velocity = glm::vec3(5, 10, -1);
    handleWallCollisionZ(&p6, h, boxWidth, elasticity);
    assert(std::abs(p6.velocity.x - 5.0f) < EPSILON && "X velocity must not change");
    assert(std::abs(p6.velocity.y - 10.0f) < EPSILON && "Y velocity must not change");
    
    // Test 7: At exact boundary
    Particle p7;
    p7.position = glm::vec3(1, 1, h - boxWidth);
    p7.velocity = glm::vec3(0, 0, -1);
    glm::vec3 pos7 = p7.position;
    handleWallCollisionZ(&p7, h, boxWidth, elasticity);
    assert(p7.position == pos7 && "At boundary must not change");
    
    std::cout << "All handleWallCollisionZ tests passed!" << std::endl;
    return 0;
}
