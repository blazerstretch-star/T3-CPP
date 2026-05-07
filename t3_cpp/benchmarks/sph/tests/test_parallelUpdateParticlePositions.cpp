#include "../src/sph_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    const float EPSILON = 0.001f;
    const size_t count = 5;
    Particle particles[count];
    float h = 0.5f;
    float boxWidth = 8.0f;
    float elasticity = 0.5f;
    float g = -9.8f;
    float deltaTime = 0.01f;
    
    // Test 1: Basic position update with gravity
    for (size_t i = 0; i < count; i++) {
        particles[i].position = glm::vec3(i * 0.5f, 1.0f, 0);
        particles[i].velocity = glm::vec3(0, 0, 0);
        particles[i].force = glm::vec3(0, 0, 0);
        particles[i].density = 1000.0f;
    }
    glm::vec3 oldPos = particles[0].position;
    
    parallelUpdateParticlePositions(particles, count, 0, count,
                                    h, boxWidth, elasticity, g, deltaTime);
    
    assert(particles[0].position.y < oldPos.y && "Gravity must pull particle down");
    float expected_vel_y = g * deltaTime;
    assert(std::abs(particles[0].velocity.y - expected_vel_y) < EPSILON && "Velocity Y mismatch");
    
    // Test 2: Partial range update
    Particle particles2[5];
    for (size_t i = 0; i < 5; i++) {
        particles2[i].position = glm::vec3(0, 1.0f, 0);
        particles2[i].velocity = glm::vec3(0, 0, 0);
        particles2[i].force = glm::vec3(0, 0, 0);
        particles2[i].density = 1000.0f;
    }
    glm::vec3 pos2_0 = particles2[0].position;
    glm::vec3 pos2_4 = particles2[4].position;
    
    parallelUpdateParticlePositions(particles2, 5, 1, 4,
                                    h, boxWidth, elasticity, g, deltaTime);
    assert(particles2[0].position == pos2_0 && "Index 0 must not be updated");
    assert(particles2[1].position.y < 1.0f && "Index 1 must be updated");
    assert(particles2[2].position.y < 1.0f && "Index 2 must be updated");
    assert(particles2[3].position.y < 1.0f && "Index 3 must be updated");
    assert(particles2[4].position == pos2_4 && "Index 4 must not be updated");
    
    // Test 3: Ground collision
    Particle particles3[1];
    particles3[0].position = glm::vec3(0, 0.3f, 0);
    particles3[0].velocity = glm::vec3(0, -5, 0);
    particles3[0].force = glm::vec3(0, 0, 0);
    particles3[0].density = 1000.0f;
    
    parallelUpdateParticlePositions(particles3, 1, 0, 1,
                                    h, boxWidth, elasticity, g, deltaTime);
    assert(particles3[0].position.y >= h && "Must be above ground after collision");
    assert(particles3[0].velocity.y > 0 && "Velocity must reverse after ground collision");
    
    // Test 4: Wall collision X
    Particle particles4[1];
    particles4[0].position = glm::vec3(-8.0f, 1, 0);
    particles4[0].velocity = glm::vec3(-5, 0, 0);
    particles4[0].force = glm::vec3(0, 0, 0);
    particles4[0].density = 1000.0f;
    
    parallelUpdateParticlePositions(particles4, 1, 0, 1,
                                    h, boxWidth, elasticity, g, deltaTime);
    assert(particles4[0].velocity.x > 0 && "X velocity must reverse after wall collision");
    
    // Test 5: Wall collision Z
    Particle particles5[1];
    particles5[0].position = glm::vec3(0, 1, -8.0f);
    particles5[0].velocity = glm::vec3(0, 0, -5);
    particles5[0].force = glm::vec3(0, 0, 0);
    particles5[0].density = 1000.0f;
    
    parallelUpdateParticlePositions(particles5, 1, 0, 1,
                                    h, boxWidth, elasticity, g, deltaTime);
    assert(particles5[0].velocity.z > 0 && "Z velocity must reverse after wall collision");
    
    // Test 6: Force acceleration
    Particle particles6[1];
    particles6[0].position = glm::vec3(0, 1, 0);
    particles6[0].velocity = glm::vec3(0, 0, 0);
    particles6[0].force = glm::vec3(1000, 0, 0);
    particles6[0].density = 1000.0f;
    
    parallelUpdateParticlePositions(particles6, 1, 0, 1,
                                    h, boxWidth, elasticity, 0, deltaTime);
    float expected_vel_x = (1000.0f / 1000.0f) * deltaTime;
    assert(std::abs(particles6[0].velocity.x - expected_vel_x) < EPSILON && "Force acceleration mismatch");
    
    // Test 7: Multiple particles independent
    Particle particles7[3];
    particles7[0].position = glm::vec3(0, 1, 0);
    particles7[1].position = glm::vec3(1, 1, 0);
    particles7[2].position = glm::vec3(2, 1, 0);
    for (size_t i = 0; i < 3; i++) {
        particles7[i].velocity = glm::vec3(0, 0, 0);
        particles7[i].force = glm::vec3(0, 0, 0);
        particles7[i].density = 1000.0f;
    }
    
    parallelUpdateParticlePositions(particles7, 3, 0, 3,
                                    h, boxWidth, elasticity, g, deltaTime);
    for (size_t i = 0; i < 3; i++) {
        assert(particles7[i].position.y < 1.0f && "All particles must fall");
    }
    
    std::cout << "All parallelUpdateParticlePositions tests passed!" << std::endl;
    return 0;
}
