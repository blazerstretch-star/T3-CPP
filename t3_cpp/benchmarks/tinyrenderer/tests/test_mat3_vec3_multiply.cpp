#include "../src/tinyrenderer_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace TinyRenderer;

int main() {
    mat3 identity = {{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}};
    vec3 v = {1, 2, 3};
    vec3 result = mat3_vec3_multiply(identity, v);
    assert(result.x == 1.0 && result.y == 2.0 && result.z == 3.0);
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
