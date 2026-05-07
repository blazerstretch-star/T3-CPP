#include "../src/tinyrenderer_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace TinyRenderer;

int main() {
    // Test 1: Standard vector
    vec3 v1 = {3, 4, 0};
    vec3 result1 = vec3_normalize(v1);
    assert(std::abs(result1.x - 0.6) < 1e-9);
    assert(std::abs(result1.y - 0.8) < 1e-9);
    assert(std::abs(result1.z - 0.0) < 1e-9);
    assert(std::abs(vec3_norm(result1) - 1.0) < 1e-9);
    
    // Test 2: Unit vector (should remain unchanged)
    vec3 v2 = {1, 0, 0};
    vec3 result2 = vec3_normalize(v2);
    assert(std::abs(result2.x - 1.0) < 1e-9);
    assert(std::abs(result2.y - 0.0) < 1e-9);
    assert(std::abs(result2.z - 0.0) < 1e-9);
    
    // Test 3: Negative values
    vec3 v3 = {-6, -8, 0};
    vec3 result3 = vec3_normalize(v3);
    assert(std::abs(result3.x + 0.6) < 1e-9);
    assert(std::abs(result3.y + 0.8) < 1e-9);
    assert(std::abs(vec3_norm(result3) - 1.0) < 1e-9);
    
    // Test 4: All components equal
    vec3 v4 = {1, 1, 1};
    vec3 result4 = vec3_normalize(v4);
    double expected = 1.0 / std::sqrt(3.0);
    assert(std::abs(result4.x - expected) < 1e-9);
    assert(std::abs(result4.y - expected) < 1e-9);
    assert(std::abs(result4.z - expected) < 1e-9);
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
