#include "../src/tinyrenderer_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace TinyRenderer;

int main() {
    vec3 eye = {0, 0, 3};
    vec3 center = {0, 0, 0};
    vec3 up = {0, 1, 0};
    mat4 result = lookat(eye, center, up);
    assert(result[3][3] == 1.0);
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
