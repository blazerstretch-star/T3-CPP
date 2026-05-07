#include "../src/cgal_kernel_functions.h"
#include <cassert>
#include <iostream>

int main() {
    using namespace CGALKernel;
    
    assert(midpoint(Point_2(0,0), Point_2(2,0)) == Point_2(1,0));
    assert(midpoint(Point_2(0,0), Point_2(0,4)) == Point_2(0,2));
    assert(midpoint(Point_2(1,1), Point_2(3,3)) == Point_2(2,2));
    assert(midpoint(Point_2(-1,-1), Point_2(1,1)) == Point_2(0,0));
    
    std::cout << "test_midpoint: PASSED\n";
    return 0;
}
