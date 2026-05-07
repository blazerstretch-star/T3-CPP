#include "../src/sph_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    const float EPSILON = 0.00001f;
    
    // Test 1: 3-4-5 triangle normalization
    glm::vec3 vec1(3, 4, 0);
    glm::vec3 norm1 = normalizeDirection(vec1);
    float len1 = glm::length(norm1);
    assert(std::abs(len1 - 1.0f) < EPSILON && "Normalized length must be 1");
    assert(std::abs(norm1.x - 0.6f) < EPSILON && "3/5 = 0.6");
    assert(std::abs(norm1.y - 0.8f) < EPSILON && "4/5 = 0.8");
    assert(std::abs(norm1.z) < EPSILON && "Z component must be 0");
    
    // Test 2: Zero vector
    glm::vec3 vec2(0, 0, 0);
    glm::vec3 norm2 = normalizeDirection(vec2);
    assert(norm2 == glm::vec3(0, 0, 0) && "Zero vector must return zero");
    
    // Test 3: Very small vector (below threshold 0.0001f) — same as Test 10, kept for clarity
    glm::vec3 vec3(0.00001f, 0, 0);
    glm::vec3 norm3 = normalizeDirection(vec3);
    assert(norm3 == glm::vec3(0, 0, 0) && "Below threshold (0.00001 < 0.0001) must return zero");
    
    // Test 4: Unit vector X
    glm::vec3 vec4(1, 0, 0);
    glm::vec3 norm4 = normalizeDirection(vec4);
    assert(std::abs(glm::length(norm4) - 1.0f) < EPSILON && "Unit vector length must be 1");
    assert(std::abs(norm4.x - 1.0f) < EPSILON && "X component must be 1");
    
    // Test 5: Unit vector Y
    glm::vec3 vec5(0, 1, 0);
    glm::vec3 norm5 = normalizeDirection(vec5);
    assert(std::abs(norm5.y - 1.0f) < EPSILON && "Y component must be 1");
    
    // Test 6: Unit vector Z
    glm::vec3 vec6(0, 0, 1);
    glm::vec3 norm6 = normalizeDirection(vec6);
    assert(std::abs(norm6.z - 1.0f) < EPSILON && "Z component must be 1");
    
    // Test 7: Negative vector
    glm::vec3 vec7(-3, -4, 0);
    glm::vec3 norm7 = normalizeDirection(vec7);
    assert(std::abs(glm::length(norm7) - 1.0f) < EPSILON && "Negative vector length must be 1");
    assert(std::abs(norm7.x + 0.6f) < EPSILON && "-3/5 = -0.6");
    assert(std::abs(norm7.y + 0.8f) < EPSILON && "-4/5 = -0.8");
    
    // Test 8: 3D vector
    glm::vec3 vec8(1, 1, 1);
    glm::vec3 norm8 = normalizeDirection(vec8);
    assert(std::abs(glm::length(norm8) - 1.0f) < EPSILON && "3D vector length must be 1");
    float expected_comp = 1.0f / std::sqrt(3.0f);
    assert(std::abs(norm8.x - expected_comp) < EPSILON && "1/sqrt(3) component");
    assert(std::abs(norm8.y - expected_comp) < EPSILON && "1/sqrt(3) component");
    assert(std::abs(norm8.z - expected_comp) < EPSILON && "1/sqrt(3) component");
    
    // Test 9: Large magnitude vector
    glm::vec3 vec9(300, 400, 0);
    glm::vec3 norm9 = normalizeDirection(vec9);
    assert(std::abs(glm::length(norm9) - 1.0f) < EPSILON && "Large vector length must be 1");
    assert(std::abs(norm9.x - 0.6f) < EPSILON && "Direction preserved");
    assert(std::abs(norm9.y - 0.8f) < EPSILON && "Direction preserved");

    // Test 10: Just below threshold (0.0001f) — must return zero
    glm::vec3 just_below(0.00009f, 0, 0);
    glm::vec3 norm_below = normalizeDirection(just_below);
    assert(norm_below == glm::vec3(0, 0, 0) && "Just below threshold must return zero");

    // Test 11: Just above threshold — must normalize
    glm::vec3 just_above(0.001f, 0, 0);
    glm::vec3 norm_above = normalizeDirection(just_above);
    assert(std::abs(glm::length(norm_above) - 1.0f) < EPSILON && "Just above threshold must normalize");

    std::cout << "All normalizeDirection tests passed!" << std::endl;
    return 0;
}
