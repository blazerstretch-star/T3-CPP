#include "../src/cgal_kernel_functions.h"
#include <cassert>
#include <iostream>

void test_point_in_polygon() {
    using namespace CGALKernel;
    
    // Test 1: Square polygon
    std::vector<Point_2> square = {
        Point_2(0,0), Point_2(4,0), Point_2(4,4), Point_2(0,4)
    };
    
    // Inside tests
    assert(point_in_polygon(square, Point_2(2,2)) == CGAL::ON_BOUNDED_SIDE);
    assert(point_in_polygon(square, Point_2(1,1)) == CGAL::ON_BOUNDED_SIDE);
    assert(point_in_polygon(square, Point_2(3,3)) == CGAL::ON_BOUNDED_SIDE);
    
    // Outside tests
    assert(point_in_polygon(square, Point_2(5,5)) == CGAL::ON_UNBOUNDED_SIDE);
    assert(point_in_polygon(square, Point_2(-1,2)) == CGAL::ON_UNBOUNDED_SIDE);
    assert(point_in_polygon(square, Point_2(2,5)) == CGAL::ON_UNBOUNDED_SIDE);
    assert(point_in_polygon(square, Point_2(-1,-1)) == CGAL::ON_UNBOUNDED_SIDE);
    
    // Boundary tests - vertices
    assert(point_in_polygon(square, Point_2(0,0)) == CGAL::ON_BOUNDARY);
    assert(point_in_polygon(square, Point_2(4,0)) == CGAL::ON_BOUNDARY);
    assert(point_in_polygon(square, Point_2(4,4)) == CGAL::ON_BOUNDARY);
    assert(point_in_polygon(square, Point_2(0,4)) == CGAL::ON_BOUNDARY);
    
    // Boundary tests - edges
    assert(point_in_polygon(square, Point_2(2,0)) == CGAL::ON_BOUNDARY);
    assert(point_in_polygon(square, Point_2(4,2)) == CGAL::ON_BOUNDARY);
    assert(point_in_polygon(square, Point_2(2,4)) == CGAL::ON_BOUNDARY);
    assert(point_in_polygon(square, Point_2(0,2)) == CGAL::ON_BOUNDARY);
    
    // Test 2: Triangle
    std::vector<Point_2> triangle = {
        Point_2(0,0), Point_2(4,0), Point_2(2,3)
    };
    assert(point_in_polygon(triangle, Point_2(2,1)) == CGAL::ON_BOUNDED_SIDE);
    assert(point_in_polygon(triangle, Point_2(2,4)) == CGAL::ON_UNBOUNDED_SIDE);
    assert(point_in_polygon(triangle, Point_2(0,0)) == CGAL::ON_BOUNDARY);
    assert(point_in_polygon(triangle, Point_2(2,0)) == CGAL::ON_BOUNDARY);
    
    std::cout << "test_point_in_polygon: PASSED\n";
}

int main() {
    test_point_in_polygon();
    return 0;
}
