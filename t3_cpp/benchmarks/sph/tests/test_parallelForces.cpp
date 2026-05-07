#include "../src/sph_functions.h"
#include <cassert>
#include <iostream>
#include <algorithm>
#include <cmath>

int main() {
    const float EPSILON = 0.01f;
    const size_t count = 5;
    Particle particles[count];
    float h = 0.5f;
    float h2 = h * h;
    float mass = 1.0f;
    float viscosity = 0.01f;
    float spikyGrad = -100.0f;
    float spikyLap = 100.0f;
    
    // Test 1: Initialize and calculate forces
    for (size_t i = 0; i < count; i++) {
        particles[i].position = glm::vec3(i * 0.1f, 0, 0);
        particles[i].velocity = glm::vec3(i * 0.5f, 0, 0);
        particles[i].pressure = 1000.0f + i * 10.0f;
        particles[i].density = 1000.0f;
        particles[i].hash = getHash(getCell(&particles[i], h));
        particles[i].force = glm::vec3(0, 0, 0);
    }
    std::sort(particles, particles + count,
              [](const Particle& a, const Particle& b) { return a.hash < b.hash; });
    uint32_t* table = createNeighborTable(particles, count);
    
    parallelForces(particles, count, 0, count, table,
                   mass, viscosity, spikyGrad, spikyLap, h, h2);
    
    // Test 2: At least some particles must have non-zero forces
    bool hasForce = false;
    for (size_t i = 0; i < count; i++) {
        if (glm::length(particles[i].force) > EPSILON) {
            hasForce = true;
            break;
        }
    }
    assert(hasForce && "At least one particle must have non-zero force");
    
    // Test 3: Partial range calculation
    Particle particles2[5];
    for (size_t i = 0; i < 5; i++) {
        particles2[i].position = glm::vec3(i * 0.1f, 0, 0);
        particles2[i].velocity = glm::vec3(i * 0.5f, 0, 0);
        particles2[i].pressure = 1000.0f;
        particles2[i].density = 1000.0f;
        particles2[i].hash = getHash(getCell(&particles2[i], h));
        particles2[i].force = glm::vec3(99, 99, 99);
    }
    std::sort(particles2, particles2 + 5,
              [](const Particle& a, const Particle& b) { return a.hash < b.hash; });
    uint32_t* table2 = createNeighborTable(particles2, 5);
    
    parallelForces(particles2, 5, 1, 4, table2,
                   mass, viscosity, spikyGrad, spikyLap, h, h2);
    assert(particles2[0].force == glm::vec3(99, 99, 99) && "Index 0 must not be calculated");
    assert(particles2[4].force == glm::vec3(99, 99, 99) && "Index 4 must not be calculated");
    
    // Test 4: Forces reset to zero before calculation
    Particle particles3[3];
    for (size_t i = 0; i < 3; i++) {
        particles3[i].position = glm::vec3(i * 0.1f, 0, 0);
        particles3[i].velocity = glm::vec3(0, 0, 0);
        particles3[i].pressure = 1000.0f;
        particles3[i].density = 1000.0f;
        particles3[i].hash = getHash(getCell(&particles3[i], h));
        particles3[i].force = glm::vec3(100, 100, 100);
    }
    std::sort(particles3, particles3 + 3,
              [](const Particle& a, const Particle& b) { return a.hash < b.hash; });
    uint32_t* table3 = createNeighborTable(particles3, 3);
    
    parallelForces(particles3, 3, 0, 3, table3,
                   mass, viscosity, spikyGrad, spikyLap, h, h2);
    for (size_t i = 0; i < 3; i++) {
        assert(particles3[i].force != glm::vec3(100, 100, 100) && "Force must be reset");
    }
    
    // Test 5: Isolated particle has zero force
    Particle particles4[1];
    particles4[0].position = glm::vec3(0, 0, 0);
    particles4[0].velocity = glm::vec3(1, 1, 1);
    particles4[0].pressure = 1000.0f;
    particles4[0].density = 1000.0f;
    particles4[0].hash = getHash(getCell(&particles4[0], h));
    particles4[0].force = glm::vec3(0, 0, 0);
    uint32_t* table4 = createNeighborTable(particles4, 1);
    
    parallelForces(particles4, 1, 0, 1, table4,
                   mass, viscosity, spikyGrad, spikyLap, h, h2);
    assert(glm::length(particles4[0].force) < EPSILON && "Isolated particle must have zero force");
    
    // Test 6: Particles far apart have zero force
    Particle particles5[2];
    particles5[0].position = glm::vec3(0, 0, 0);
    particles5[1].position = glm::vec3(10, 0, 0);
    for (size_t i = 0; i < 2; i++) {
        particles5[i].velocity = glm::vec3(0, 0, 0);
        particles5[i].pressure = 1000.0f;
        particles5[i].density = 1000.0f;
        particles5[i].hash = getHash(getCell(&particles5[i], h));
        particles5[i].force = glm::vec3(0, 0, 0);
    }
    std::sort(particles5, particles5 + 2,
              [](const Particle& a, const Particle& b) { return a.hash < b.hash; });
    uint32_t* table5 = createNeighborTable(particles5, 2);
    
    parallelForces(particles5, 2, 0, 2, table5,
                   mass, viscosity, spikyGrad, spikyLap, h, h2);
    assert(glm::length(particles5[0].force) < EPSILON && "Far particles must have zero force");
    
    free(table);
    free(table2);
    free(table3);
    free(table4);
    free(table5);
    std::cout << "All parallelForces tests passed!" << std::endl;
    return 0;
}
