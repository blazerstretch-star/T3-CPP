#include "../src/tinyrenderer_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace TinyRenderer;

int main() {
    vec3 a = {4, 5, 6};
    vec3 b = {1, 2, 3};
    vec3 result = vec3_subtract(a, b);
    assert(result.x == 3.0 && result.y == 3.0 && result.z == 3.0);
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
