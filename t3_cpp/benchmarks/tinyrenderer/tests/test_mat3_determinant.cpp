#include "../src/tinyrenderer_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace TinyRenderer;

int main() {
    // Test 1: Identity matrix (det = 1)
    mat3 identity = {{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}};
    assert(std::abs(mat3_determinant(identity) - 1.0) < 1e-9);
    
    // Test 2: Zero matrix (det = 0)
    mat3 zero = {{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}};
    assert(std::abs(mat3_determinant(zero) - 0.0) < 1e-9);
    
    // Test 3: Known determinant
    mat3 m1 = {{{1, 2, 3}, {0, 1, 4}, {5, 6, 0}}};
    assert(std::abs(mat3_determinant(m1) - 1.0) < 1e-9);
    
    // Test 4: Scaled identity (det = scale^3)
    mat3 scaled = {{{2, 0, 0}, {0, 2, 0}, {0, 0, 2}}};
    assert(std::abs(mat3_determinant(scaled) - 8.0) < 1e-9);
    
    // Test 5: Singular matrix (det = 0)
    mat3 singular = {{{1, 2, 3}, {2, 4, 6}, {3, 6, 9}}};
    assert(std::abs(mat3_determinant(singular) - 0.0) < 1e-9);
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
