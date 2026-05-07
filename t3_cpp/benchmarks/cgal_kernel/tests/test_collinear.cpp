#include "../src/cgal_kernel_functions.h"
#include <cassert>
#include <iostream>

int main() {
    using namespace CGALKernel;
    
    assert(collinear(Point_2(0,0), Point_2(1,1), Point_2(2,2)));
    assert(collinear(Point_2(0,0), Point_2(1,0), Point_2(2,0)));
    assert(!collinear(Point_2(0,0), Point_2(1,0), Point_2(0,1)));
    assert(collinear(Point_2(0,0), Point_2(0,0), Point_2(0,0)));
    
    std::cout << "test_collinear: PASSED\n";
    return 0;
}
