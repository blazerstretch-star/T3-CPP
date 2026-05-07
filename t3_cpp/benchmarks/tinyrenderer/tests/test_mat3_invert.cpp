#include "../src/tinyrenderer_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace TinyRenderer;

int main() {
    mat3 identity = {{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}};
    mat3 result = mat3_invert(identity);
    assert(result[0][0] == 1.0 && result[1][1] == 1.0 && result[2][2] == 1.0);
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
