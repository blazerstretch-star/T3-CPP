#include "../src/tinyrenderer_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace TinyRenderer;

int main() {
    mat4 identity = {{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}};
    mat4 m = {{{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 16}}};
    mat4 result = mat4_multiply(identity, m);
    assert(result[0][0] == 1.0 && result[1][1] == 6.0);
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
