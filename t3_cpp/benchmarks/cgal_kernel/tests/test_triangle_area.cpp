#include "../src/cgal_kernel_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    using namespace CGALKernel;
    
    FT area = triangle_area(Point_2(0,0), Point_2(2,0), Point_2(1,1));
    assert(std::abs(CGAL::to_double(area) - 1.0) < 0.001);
    
    area = triangle_area(Point_2(0,0), Point_2(4,0), Point_2(0,3));
    assert(std::abs(CGAL::to_double(area) - 6.0) < 0.001);
    
    area = triangle_area(Point_2(0,0), Point_2(1,0), Point_2(2,0));
    assert(std::abs(CGAL::to_double(area)) < 0.001);
    
    std::cout << "test_triangle_area: PASSED\n";
    return 0;
}
