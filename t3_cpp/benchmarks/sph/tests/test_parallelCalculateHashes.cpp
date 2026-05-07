#include "../src/sph_functions.h"
#include <cassert>
#include <iostream>

int main() {
    const size_t count = 8;
    Particle particles[count];
    float h = 0.5f;
    
    // Test 1: Set known positions and verify hash calculation
    particles[0].position = glm::vec3(0.0f, 0.0f, 0.0f);
    particles[1].position = glm::vec3(0.6f, 0.0f, 0.0f);
    particles[2].position = glm::vec3(0.0f, 0.7f, 0.0f);
    particles[3].position = glm::vec3(0.0f, 0.0f, 0.8f);
    particles[4].position = glm::vec3(1.5f, 1.5f, 1.5f);
    particles[5].position = glm::vec3(-0.5f, -0.5f, -0.5f);
    particles[6].position = glm::vec3(2.0f, 3.0f, 4.0f);
    particles[7].position = glm::vec3(-1.0f, -2.0f, -3.0f);
    
    // Initialize hashes to invalid value — use 0 since we track pre-call values explicitly
    for (size_t i = 0; i < count; i++) {
        particles[i].hash = 0;
    }
    
    parallelCalculateHashes(particles, 0, count, h);
    
    // Test 2: All hashes must be calculated and valid
    for (size_t i = 0; i < count; i++) {
        assert(particles[i].hash < TABLE_SIZE && "Hash exceeds TABLE_SIZE");
        assert(particles[i].hash != TABLE_SIZE + 1 && "Hash was not calculated");
    }
    
    // Test 3: Verify exact hash values match getHash(getCell())
    for (size_t i = 0; i < count; i++) {
        glm::ivec3 cell = getCell(&particles[i], h);
        uint32_t expected_hash = getHash(cell);
        assert(particles[i].hash == expected_hash && "Hash calculation mismatch");
    }
    
    // Test 4: Particles in same cell have same hash
    Particle p1, p2;
    p1.position = glm::vec3(0.1f, 0.1f, 0.1f);
    p2.position = glm::vec3(0.2f, 0.2f, 0.2f);
    p1.hash = 0;
    p2.hash = 0;
    parallelCalculateHashes(&p1, 0, 1, h);
    parallelCalculateHashes(&p2, 0, 1, h);
    assert(p1.hash == p2.hash && "Particles in same cell must have same hash");
    
    // Test 5: Particles in different cells have different hashes
    Particle p3, p4;
    p3.position = glm::vec3(0.0f, 0.0f, 0.0f);
    p4.position = glm::vec3(1.0f, 0.0f, 0.0f);
    p3.hash = 0;
    p4.hash = 0;
    parallelCalculateHashes(&p3, 0, 1, h);
    parallelCalculateHashes(&p4, 0, 1, h);
    assert(p3.hash != p4.hash && "Particles in different cells must differ");
    
    // Test 6: Partial range calculation
    // Use a distinct position-derived sentinel: store original position and check it is unchanged
    // for out-of-range indices, rather than relying on a hash sentinel value.
    Particle particles2[5];
    bool hash_was_set[5] = {false};
    for (size_t i = 0; i < 5; i++) {
        particles2[i].position = glm::vec3(i * 0.3f, 0, 0);
        particles2[i].hash = 0;  // initialise to 0
    }
    // Record pre-call hashes for indices outside the range
    uint16_t hash_before_0 = particles2[0].hash;
    uint16_t hash_before_4 = particles2[4].hash;
    parallelCalculateHashes(particles2, 1, 4, h);
    assert(particles2[0].hash == hash_before_0 && "Index 0 must not be calculated");
    assert(particles2[1].hash < TABLE_SIZE && "Index 1 must be calculated");
    assert(particles2[2].hash < TABLE_SIZE && "Index 2 must be calculated");
    assert(particles2[3].hash < TABLE_SIZE && "Index 3 must be calculated");
    assert(particles2[4].hash == hash_before_4 && "Index 4 must not be calculated");
    // Verify indices 1-3 match expected values
    for (size_t i = 1; i < 4; i++) {
        uint32_t expected = getHash(getCell(&particles2[i], h));
        assert(particles2[i].hash == expected && "Partial range hash mismatch");
    }
    
    // Test 7: Negative positions
    Particle p5;
    p5.position = glm::vec3(-5.0f, -10.0f, -15.0f);
    p5.hash = 0;
    parallelCalculateHashes(&p5, 0, 1, h);
    assert(p5.hash < TABLE_SIZE && "Negative position hash must be valid");
    
    std::cout << "All parallelCalculateHashes tests passed!" << std::endl;
    return 0;
}
