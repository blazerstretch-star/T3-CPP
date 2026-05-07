#include "../src/cgal_kernel_functions.h"
#include <cassert>
#include <iostream>

int main() {
    using namespace CGALKernel;
    
    assert(compare_y(Point_2(0,0), Point_2(0,1)) == CGAL::SMALLER);
    assert(compare_y(Point_2(0,1), Point_2(0,1)) == CGAL::EQUAL);
    assert(compare_y(Point_2(0,2), Point_2(0,1)) == CGAL::LARGER);
    
    std::cout << "test_compare_y: PASSED\n";
    return 0;
}
