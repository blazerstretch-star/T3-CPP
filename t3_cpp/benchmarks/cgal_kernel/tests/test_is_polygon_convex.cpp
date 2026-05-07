#include "../src/cgal_kernel_functions.h"
#include <cassert>
#include <iostream>

void test_is_polygon_convex() {
    using namespace CGALKernel;
    
    // Test 1: Convex square
    std::vector<Point_2> square = {
        Point_2(0,0), Point_2(2,0), Point_2(2,2), Point_2(0,2)
    };
    assert(is_polygon_convex(square));
    
    // Test 2: Convex triangle
    std::vector<Point_2> triangle = {
        Point_2(0,0), Point_2(2,0), Point_2(1,2)
    };
    assert(is_polygon_convex(triangle));
    
    // Test 3: Concave polygon (star shape)
    std::vector<Point_2> concave = {
        Point_2(0,0), Point_2(2,0), Point_2(2,2), Point_2(1,1), Point_2(0,2)
    };
    assert(!is_polygon_convex(concave));
    
    // Test 4: Convex hexagon
    std::vector<Point_2> hexagon = {
        Point_2(1,0), Point_2(2,0), Point_2(3,1), Point_2(2,2), Point_2(1,2), Point_2(0,1)
    };
    assert(is_polygon_convex(hexagon));
    
    // Test 5: Concave L-shape
    std::vector<Point_2> l_shape = {
        Point_2(0,0), Point_2(2,0), Point_2(2,1), Point_2(1,1), Point_2(1,2), Point_2(0,2)
    };
    assert(!is_polygon_convex(l_shape));
    
    // Test 6: Empty polygon (degenerate - considered convex)
    std::vector<Point_2> empty;
    assert(is_polygon_convex(empty));
    
    std::cout << "test_is_polygon_convex: PASSED\n";
}

int main() {
    test_is_polygon_convex();
    return 0;
}
