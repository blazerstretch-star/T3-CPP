#include "../src/cgal_kernel_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    using namespace CGALKernel;
    
    FT det = determinant(Vector_2(1,0), Vector_2(0,1));
    assert(std::abs(CGAL::to_double(det) - 1.0) < 0.001);
    
    det = determinant(Vector_2(2,3), Vector_2(4,5));
    assert(std::abs(CGAL::to_double(det) + 2.0) < 0.001);
    
    det = determinant(Vector_2(1,2), Vector_2(2,4));
    assert(std::abs(CGAL::to_double(det)) < 0.001);
    
    std::cout << "test_determinant: PASSED\n";
    return 0;
}
