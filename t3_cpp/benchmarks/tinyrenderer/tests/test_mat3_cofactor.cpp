#include "../src/tinyrenderer_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace TinyRenderer;

int main() {
    mat3 m = {{{1, 2, 3}, {0, 1, 4}, {5, 6, 0}}};
    double result = mat3_cofactor(m, 0, 0);
    assert(result == -24.0);
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
