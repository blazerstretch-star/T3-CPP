#include "../src/tinyrenderer_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace TinyRenderer;

int main() {
    vec3 v = {3, 4, 0};
    double result = vec3_norm(v);
    assert(result == 5.0);
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
