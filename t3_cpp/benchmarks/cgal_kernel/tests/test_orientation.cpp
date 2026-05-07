#include "../src/cgal_kernel_functions.h"
#include <cassert>
#include <iostream>

int main() {
    using namespace CGALKernel;
    
    assert(orientation(Point_2(0,0), Point_2(1,0), Point_2(1,1)) == CGAL::LEFT_TURN);
    assert(orientation(Point_2(0,0), Point_2(1,0), Point_2(1,-1)) == CGAL::RIGHT_TURN);
    assert(orientation(Point_2(0,0), Point_2(1,0), Point_2(2,0)) == CGAL::COLLINEAR);
    
    std::cout << "test_orientation: PASSED\n";
    return 0;
}
