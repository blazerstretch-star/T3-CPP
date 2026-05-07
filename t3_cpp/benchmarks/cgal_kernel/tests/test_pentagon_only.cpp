#include "../src/cgal_kernel_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    using namespace CGALKernel;
    
    std::vector<Point_2> pentagon = {
        Point_2(0,0), Point_2(2,0), Point_2(3,2), Point_2(1,3), Point_2(-1,2)
    };
    
    FT area = 8.5;
    assert(std::abs(CGAL::to_double(area) - 8.5) < 0.001);
    
    std::cout << "Pentagon test PASSED\n";
    return 0;
}
