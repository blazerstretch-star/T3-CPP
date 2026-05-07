#include "../src/tinyrenderer_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace TinyRenderer;

int main() {
    vec2 p = {0.5, 0.5};
    vec2 a = {0, 0};
    vec2 b = {1, 0};
    vec2 c = {0, 1};
    vec3 result = barycentric(p, a, b, c);
    assert(std::abs(result.x + result.y + result.z - 1.0) < 1e-6);
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
