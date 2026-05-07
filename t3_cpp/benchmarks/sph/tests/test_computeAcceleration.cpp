#include "../src/sph_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    const float EPSILON = 0.00001f;
    
    // Test 1: Basic acceleration calculation
    glm::vec3 force(100, 0, 0);
    float density = 1000.0f;
    float g = -9.8f;
    glm::vec3 accel = computeAcceleration(force, density, g);
    assert(std::abs(accel.x - 0.1f) < EPSILON && "Force/density = 100/1000 = 0.1");
    assert(std::abs(accel.y - g) < EPSILON && "Y component must be g");
    assert(std::abs(accel.z) < EPSILON && "Z component must be 0");
    
    // Test 2: Zero force
    glm::vec3 force2(0, 0, 0);
    glm::vec3 accel2 = computeAcceleration(force2, density, g);
    assert(std::abs(accel2.x) < EPSILON && "Zero force X = 0");
    assert(std::abs(accel2.y - g) < EPSILON && "Gravity still applies");
    assert(std::abs(accel2.z) < EPSILON && "Zero force Z = 0");
    
    // Test 3: Different density
    glm::vec3 force3(500, 0, 0);
    glm::vec3 accel3 = computeAcceleration(force3, 500.0f, g);
    assert(std::abs(accel3.x - 1.0f) < EPSILON && "500/500 = 1.0");
    
    // Test 4: 3D force
    glm::vec3 force4(100, 200, 300);
    glm::vec3 accel4 = computeAcceleration(force4, 1000.0f, g);
    assert(std::abs(accel4.x - 0.1f) < EPSILON && "X: 100/1000 = 0.1");
    assert(std::abs(accel4.y - (0.2f + g)) < EPSILON && "Y: 200/1000 + g");
    assert(std::abs(accel4.z - 0.3f) < EPSILON && "Z: 300/1000 = 0.3");
    
    // Test 5: Negative force
    glm::vec3 force5(-100, 0, 0);
    glm::vec3 accel5 = computeAcceleration(force5, 1000.0f, g);
    assert(std::abs(accel5.x + 0.1f) < EPSILON && "Negative force: -100/1000 = -0.1");
    
    // Test 6: Different gravity
    glm::vec3 accel6 = computeAcceleration(force, density, -20.0f);
    assert(std::abs(accel6.y + 20.0f) < EPSILON && "Different gravity value");
    
    // Test 7: Zero gravity
    glm::vec3 accel7 = computeAcceleration(force, density, 0.0f);
    assert(std::abs(accel7.y) < EPSILON && "Zero gravity");
    assert(std::abs(accel7.x - 0.1f) < EPSILON && "Force component unchanged");
    
    // Test 8: High density (low acceleration)
    glm::vec3 force8(1000, 0, 0);
    glm::vec3 accel8 = computeAcceleration(force8, 10000.0f, g);
    assert(std::abs(accel8.x - 0.1f) < EPSILON && "1000/10000 = 0.1");
    
    std::cout << "All computeAcceleration tests passed!" << std::endl;
    return 0;
}
