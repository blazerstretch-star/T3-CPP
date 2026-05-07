#include "../src/sph_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    const float EPSILON = 0.00001f;
    
    // Test 1: 3-4-5 triangle
    glm::vec3 p1(0, 0, 0);
    glm::vec3 p2(3, 4, 0);
    float dist2 = calculateDistanceSquared(p1, p2);
    assert(std::abs(dist2 - 25.0f) < EPSILON && "3^2 + 4^2 = 25");
    
    // Test 2: Same point
    float dist2_same = calculateDistanceSquared(p1, p1);
    assert(dist2_same == 0 && "Same point must give zero distance");
    
    // Test 3: Unit distance X-axis
    glm::vec3 p3(1, 0, 0);
    float dist2_x = calculateDistanceSquared(p1, p3);
    assert(std::abs(dist2_x - 1.0f) < EPSILON && "Unit X distance squared = 1");
    
    // Test 4: Unit distance Y-axis
    glm::vec3 p4(0, 1, 0);
    float dist2_y = calculateDistanceSquared(p1, p4);
    assert(std::abs(dist2_y - 1.0f) < EPSILON && "Unit Y distance squared = 1");
    
    // Test 5: Unit distance Z-axis
    glm::vec3 p5(0, 0, 1);
    float dist2_z = calculateDistanceSquared(p1, p5);
    assert(std::abs(dist2_z - 1.0f) < EPSILON && "Unit Z distance squared = 1");
    
    // Test 6: 3D distance
    glm::vec3 p6(1, 1, 1);
    float dist2_3d = calculateDistanceSquared(p1, p6);
    assert(std::abs(dist2_3d - 3.0f) < EPSILON && "1^2 + 1^2 + 1^2 = 3");
    
    // Test 7: Negative coordinates
    glm::vec3 p7(-3, -4, 0);
    float dist2_neg = calculateDistanceSquared(p1, p7);
    assert(std::abs(dist2_neg - 25.0f) < EPSILON && "Negative coords: (-3)^2 + (-4)^2 = 25");
    
    // Test 8: Symmetry
    float dist2_ab = calculateDistanceSquared(p1, p2);
    float dist2_ba = calculateDistanceSquared(p2, p1);
    assert(std::abs(dist2_ab - dist2_ba) < EPSILON && "Distance must be symmetric");
    
    // Test 9: Large coordinates
    glm::vec3 p8(100, 100, 100);
    glm::vec3 p9(101, 101, 101);
    float dist2_large = calculateDistanceSquared(p8, p9);
    assert(std::abs(dist2_large - 3.0f) < EPSILON && "Large coords: 1^2 + 1^2 + 1^2 = 3");

    // Test 10: Neither point is origin — catches (a·a - b·b) style wrong formula
    // (1-4)^2 + (2-6)^2 + (3-3)^2 = 9 + 16 + 0 = 25
    glm::vec3 pa(1, 2, 3), pb(4, 6, 3);
    assert(std::abs(calculateDistanceSquared(pa, pb) - 25.0f) < EPSILON && "Non-origin pair: 9+16+0=25");

    std::cout << "All calculateDistanceSquared tests passed!" << std::endl;
    return 0;
}
