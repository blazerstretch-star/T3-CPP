#include "../src/cgal_kernel_functions.h"
#include <cassert>
#include <iostream>

int main() {
    using namespace CGALKernel;
    
    assert(left_turn(Point_2(0,0), Point_2(1,0), Point_2(1,1)));
    assert(!left_turn(Point_2(0,0), Point_2(1,0), Point_2(1,-1)));
    assert(!left_turn(Point_2(0,0), Point_2(1,0), Point_2(2,0)));
    
    std::cout << "test_left_turn: PASSED\n";
    return 0;
}
