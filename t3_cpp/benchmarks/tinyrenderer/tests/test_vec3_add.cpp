#include "../src/tinyrenderer_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace TinyRenderer;

int main() {
    vec3 a = {1, 2, 3};
    vec3 b = {4, 5, 6};
    vec3 result = vec3_add(a, b);
    assert(result.x == 5.0 && result.y == 7.0 && result.z == 9.0);
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
