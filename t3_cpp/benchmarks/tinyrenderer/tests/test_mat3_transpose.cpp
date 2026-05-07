#include "../src/tinyrenderer_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace TinyRenderer;

int main() {
    mat3 m = {{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}};
    mat3 result = mat3_transpose(m);
    assert(result[0][0] == 1.0 && result[0][1] == 4.0 && result[1][0] == 2.0);
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
