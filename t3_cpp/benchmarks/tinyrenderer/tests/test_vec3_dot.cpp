#include "../src/tinyrenderer_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace TinyRenderer;

int main() {
    // Test 1: Basic positive values
    vec3 a = {1, 2, 3};
    vec3 b = {4, 5, 6};
    assert(vec3_dot(a, b) == 32.0);
    
    // Test 2: Zero vector
    vec3 zero = {0, 0, 0};
    assert(vec3_dot(a, zero) == 0.0);
    
    // Test 3: Negative values
    vec3 c = {-1, -2, -3};
    assert(vec3_dot(a, c) == -14.0);
    
    // Test 4: Orthogonal vectors (should be 0)
    vec3 d = {1, 0, 0};
    vec3 e = {0, 1, 0};
    assert(vec3_dot(d, e) == 0.0);
    
    // Test 5: Same vector (should be squared magnitude)
    vec3 f = {3, 4, 0};
    assert(vec3_dot(f, f) == 25.0);
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
