#include "../src/tinyrenderer_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace TinyRenderer;

int main() {
    vec2 a = {0, 0};
    vec2 b = {1, 0};
    vec2 c = {0, 1};
    bool result = is_backface(a, b, c);
    assert(result == false);
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
