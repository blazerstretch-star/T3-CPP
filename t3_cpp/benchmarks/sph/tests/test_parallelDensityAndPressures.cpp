#include "../src/sph_functions.h"
#include <cassert>
#include <iostream>
#include <algorithm>
#include <cmath>

int main() {
    const float EPSILON = 0.1f;
    const size_t count = 5;
    Particle particles[count];
    float h = 0.5f;
    float h2 = h * h;
    float mass = 1.0f;
    float poly6 = 100.0f;
    float selfDens = 10.0f;
    float restDensity = 1000.0f;
    float gasConstant = 2000.0f;
    
    // Test 1: Initialize and calculate
    for (size_t i = 0; i < count; i++) {
        particles[i].position = glm::vec3(i * 0.1f, 0, 0);
        particles[i].hash = getHash(getCell(&particles[i], h));
        particles[i].density = 0;
        particles[i].pressure = 0;
    }
    std::sort(particles, particles + count, 
              [](const Particle& a, const Particle& b) { return a.hash < b.hash; });
    uint32_t* table = createNeighborTable(particles, count);
    
    parallelDensityAndPressures(particles, count, 0, count, table,
                                 mass, poly6, h, h2, selfDens, restDensity, gasConstant);
    
    // Test 2: All densities must be calculated and positive
    for (size_t i = 0; i < count; i++) {
        assert(particles[i].density > 0 && "Density must be positive");
        assert(particles[i].density >= selfDens && "Density must include self density");
    }
    
    // Test 3: Pressure calculation verification
    for (size_t i = 0; i < count; i++) {
        float expected_pressure = gasConstant * (particles[i].density - restDensity);
        assert(std::abs(particles[i].pressure - expected_pressure) < EPSILON && "Pressure calculation mismatch");
    }
    
    // Test 4: Partial range calculation
    Particle particles2[5];
    for (size_t i = 0; i < 5; i++) {
        particles2[i].position = glm::vec3(i * 0.1f, 0, 0);
        particles2[i].hash = getHash(getCell(&particles2[i], h));
        particles2[i].density = 0;
        particles2[i].pressure = 0;
    }
    std::sort(particles2, particles2 + 5,
              [](const Particle& a, const Particle& b) { return a.hash < b.hash; });
    uint32_t* table2 = createNeighborTable(particles2, 5);
    
    parallelDensityAndPressures(particles2, 5, 1, 4, table2,
                                 mass, poly6, h, h2, selfDens, restDensity, gasConstant);
    assert(particles2[0].density == 0 && "Index 0 must not be calculated");
    assert(particles2[1].density > 0 && "Index 1 must be calculated");
    assert(particles2[2].density > 0 && "Index 2 must be calculated");
    assert(particles2[3].density > 0 && "Index 3 must be calculated");
    assert(particles2[4].density == 0 && "Index 4 must not be calculated");
    
    // Test 5: Isolated particle (only self density)
    Particle particles3[1];
    particles3[0].position = glm::vec3(0, 0, 0);
    particles3[0].hash = getHash(getCell(&particles3[0], h));
    particles3[0].density = 0;
    particles3[0].pressure = 0;
    uint32_t* table3 = createNeighborTable(particles3, 1);
    
    parallelDensityAndPressures(particles3, 1, 0, 1, table3,
                                 mass, poly6, h, h2, selfDens, restDensity, gasConstant);
    assert(std::abs(particles3[0].density - selfDens) < EPSILON && "Isolated particle must have only self density");
    
    // Test 6: Verify neighbor contributions
    Particle particles4[3];
    particles4[0].position = glm::vec3(0, 0, 0);
    particles4[1].position = glm::vec3(0.1f, 0, 0);
    particles4[2].position = glm::vec3(10.0f, 0, 0);
    for (size_t i = 0; i < 3; i++) {
        particles4[i].hash = getHash(getCell(&particles4[i], h));
        particles4[i].density = 0;
        particles4[i].pressure = 0;
    }
    std::sort(particles4, particles4 + 3,
              [](const Particle& a, const Particle& b) { return a.hash < b.hash; });
    uint32_t* table4 = createNeighborTable(particles4, 3);
    
    parallelDensityAndPressures(particles4, 3, 0, 1, table4,
                                 mass, poly6, h, h2, selfDens, restDensity, gasConstant);
    assert(particles4[0].density > selfDens && "Must include neighbor contribution");
    
    free(table);
    free(table2);
    free(table3);
    free(table4);
    std::cout << "All parallelDensityAndPressures tests passed!" << std::endl;
    return 0;
}
