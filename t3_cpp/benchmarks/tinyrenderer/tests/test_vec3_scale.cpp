#include "../src/tinyrenderer_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace TinyRenderer;

int main() {
    vec3 v = {1, 2, 3};
    vec3 result = vec3_scale(v, 2.0);
    assert(result.x == 2.0 && result.y == 4.0 && result.z == 6.0);
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
