#include "../src/sph_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    const float EPSILON = 0.0001f;
    float h = 0.5f;
    float boxWidth = 8.0f;
    float elasticity = 0.5f;
    
    // Test 1: Ground collision - particle below ground
    Particle p1;
    p1.position = glm::vec3(0, 0.3f, 0);
    p1.velocity = glm::vec3(0, -1, 0);
    applyBoundaryConditions(&p1, h, boxWidth, elasticity);
    assert(p1.position.y >= h && "Particle must be at or above ground level h");
    assert(p1.velocity.y > 0 && "Velocity must reverse direction");
    assert(std::abs(p1.velocity.y - 0.5f) < EPSILON && "Velocity damping: |-(-1)| * 0.5 = 0.5");
    
    // Test 2: Left wall collision (X-axis)
    Particle p2;
    p2.position = glm::vec3(-8.0f, 1, 1);
    p2.velocity = glm::vec3(-1, 0, 0);
    applyBoundaryConditions(&p2, h, boxWidth, elasticity);
    assert(p2.velocity.x > 0 && "X velocity must reverse");
    assert(std::abs(p2.velocity.x - 0.5f) < EPSILON && "X velocity damping mismatch");
    assert(p2.position.x > h - boxWidth && "Reflected X position must be inside left boundary");
    
    // Test 3: Right wall collision (X-axis)
    Particle p3;
    p3.position = glm::vec3(8.0f, 1, 1);
    p3.velocity = glm::vec3(1, 0, 0);
    applyBoundaryConditions(&p3, h, boxWidth, elasticity);
    assert(p3.velocity.x < 0 && "X velocity must reverse for right wall");
    
    // Test 4: Front wall collision (Z-axis)
    Particle p4;
    p4.position = glm::vec3(0, 1, -8.0f);
    p4.velocity = glm::vec3(0, 0, -1);
    applyBoundaryConditions(&p4, h, boxWidth, elasticity);
    assert(p4.velocity.z > 0 && "Z velocity must reverse");
    assert(std::abs(p4.velocity.z - 0.5f) < EPSILON && "Z velocity damping mismatch");
    
    // Test 5: Back wall collision (Z-axis)
    Particle p5;
    p5.position = glm::vec3(0, 1, 8.0f);
    p5.velocity = glm::vec3(0, 0, 1);
    applyBoundaryConditions(&p5, h, boxWidth, elasticity);
    assert(p5.velocity.z < 0 && "Z velocity must reverse for back wall");
    
    // Test 6: No collision - particle in safe zone
    Particle p6;
    p6.position = glm::vec3(0, 2, 0);
    p6.velocity = glm::vec3(1, 1, 1);
    glm::vec3 old_pos = p6.position;
    glm::vec3 old_vel = p6.velocity;
    applyBoundaryConditions(&p6, h, boxWidth, elasticity);
    assert(p6.position == old_pos && "Position must not change in safe zone");
    assert(p6.velocity == old_vel && "Velocity must not change in safe zone");
    
    // Test 7: Multiple simultaneous collisions (corner)
    Particle p7;
    p7.position = glm::vec3(-8.0f, 0.3f, -8.0f);
    p7.velocity = glm::vec3(-1, -1, -1);
    applyBoundaryConditions(&p7, h, boxWidth, elasticity);
    assert(p7.position.y >= h && "Y must be corrected");
    assert(p7.velocity.x > 0 && "X velocity must reverse");
    assert(p7.velocity.y > 0 && "Y velocity must reverse");
    assert(p7.velocity.z > 0 && "Z velocity must reverse");

    // Test 8: Corner — verify positions are corrected, not just velocities
    Particle pc;
    pc.position = glm::vec3(-8.0f, 0.3f, -8.0f);
    pc.velocity = glm::vec3(-1, -1, -1);
    applyBoundaryConditions(&pc, h, boxWidth, elasticity);
    assert(pc.position.y >= h              && "Corner: Y position corrected");
    assert(pc.position.x > h - boxWidth   && "Corner: X position corrected");
    assert(pc.position.z > h - boxWidth   && "Corner: Z position corrected");

    // Test 9: Different elasticity
    Particle p8;
    p8.position = glm::vec3(0, 0.3f, 0);
    p8.velocity = glm::vec3(0, -2, 0);
    applyBoundaryConditions(&p8, h, boxWidth, 0.8f);
    assert(std::abs(p8.velocity.y - 1.6f) < EPSILON && "High elasticity damping mismatch");

    std::cout << "All applyBoundaryConditions tests passed!" << std::endl;
    return 0;
}
