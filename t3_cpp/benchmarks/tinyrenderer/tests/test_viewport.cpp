#include "../src/tinyrenderer_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace TinyRenderer;

int main() {
    mat4 result = viewport(0, 0, 800, 600);
    assert(result[0][0] == 400.0 && result[1][1] == 300.0);
    assert(result[0][3] == 400.0 && result[1][3] == 300.0);
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
