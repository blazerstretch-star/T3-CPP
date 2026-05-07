#include "../src/tinyrenderer_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace TinyRenderer;

int main() {
    mat4 identity = {{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}};
    vec4 v = {1, 2, 3, 4};
    vec4 result = mat4_vec4_multiply(identity, v);
    assert(result.x == 1.0 && result.y == 2.0 && result.z == 3.0 && result.w == 4.0);
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
