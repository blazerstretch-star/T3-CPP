#include "../src/cgal_kernel_functions.h"
#include <cassert>
#include <iostream>

int main() {
    using namespace CGALKernel;
    
    std::vector<Point_2> points = {
        Point_2(3,1), Point_2(1,2), Point_2(0,0), Point_2(2,3), Point_2(0,5)
    };
    Point_2 left = leftmost_point(points);
    assert(left == Point_2(0,0));
    
    std::vector<Point_2> single = {Point_2(5,5)};
    left = leftmost_point(single);
    assert(left == Point_2(5,5));
    
    std::vector<Point_2> negative = {
        Point_2(-1,0), Point_2(-2,1), Point_2(0,0)
    };
    left = leftmost_point(negative);
    assert(left == Point_2(-2,1));
    
    std::cout << "test_leftmost_point: PASSED\n";
    return 0;
}
