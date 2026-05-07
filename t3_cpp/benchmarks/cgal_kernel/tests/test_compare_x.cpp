#include "../src/cgal_kernel_functions.h"
#include <cassert>
#include <iostream>

int main() {
    using namespace CGALKernel;
    
    assert(compare_x(Point_2(0,0), Point_2(1,0)) == CGAL::SMALLER);
    assert(compare_x(Point_2(1,0), Point_2(1,0)) == CGAL::EQUAL);
    assert(compare_x(Point_2(2,0), Point_2(1,0)) == CGAL::LARGER);
    
    std::cout << "test_compare_x: PASSED\n";
    return 0;
}
