#include "../src/cgal_kernel_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    using namespace CGALKernel;
    
    FT dist = squared_distance(Point_2(0,0), Point_2(3,4));
    assert(std::abs(CGAL::to_double(dist) - 25.0) < 0.001);
    
    dist = squared_distance(Point_2(1,1), Point_2(1,1));
    assert(std::abs(CGAL::to_double(dist)) < 0.001);
    
    dist = squared_distance(Point_2(-1,-1), Point_2(1,1));
    assert(std::abs(CGAL::to_double(dist) - 8.0) < 0.001);
    
    std::cout << "test_squared_distance: PASSED\n";
    return 0;
}
