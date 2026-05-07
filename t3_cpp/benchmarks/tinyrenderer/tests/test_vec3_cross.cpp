#include "../src/tinyrenderer_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace TinyRenderer;

int main() {
    vec3 a = {1, 0, 0};
    vec3 b = {0, 1, 0};
    vec3 result = vec3_cross(a, b);
    assert(result.x == 0.0 && result.y == 0.0 && result.z == 1.0);
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
