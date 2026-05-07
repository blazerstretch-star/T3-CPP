#include "../src/tinyrenderer_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace TinyRenderer;

int main() {
    mat4 result = perspective(3.0);
    assert(result[0][0] == 1.0 && result[1][1] == 1.0);
    assert(std::abs(result[3][2] + 1.0/3.0) < 1e-6);
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
