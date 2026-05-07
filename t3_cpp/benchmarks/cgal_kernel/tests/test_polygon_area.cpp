#include "../src/cgal_kernel_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

void test_polygon_area() {
    using namespace CGALKernel;
    
    // Test 1: Unit square (area = 1)
    std::vector<Point_2> square = {
        Point_2(0,0), Point_2(1,0), Point_2(1,1), Point_2(0,1)
    };
    FT area = polygon_area(square);
    assert(std::abs(CGAL::to_double(area) - 1.0) < 0.001);
    
    // Test 2: Triangle (base=4, height=3, area=6)
    std::vector<Point_2> triangle = {
        Point_2(0,0), Point_2(4,0), Point_2(2,3)
    };
    area = polygon_area(triangle);
    assert(std::abs(CGAL::to_double(area) - 6.0) < 0.001);
    
    // Test 3: Larger square (area = 4)
    std::vector<Point_2> big_square = {
        Point_2(0,0), Point_2(2,0), Point_2(2,2), Point_2(0,2)
    };
    area = polygon_area(big_square);
    assert(std::abs(CGAL::to_double(area) - 4.0) < 0.001);
    
    // Test 4: Empty polygon (area = 0)
    std::vector<Point_2> empty;
    area = polygon_area(empty);
    assert(std::abs(CGAL::to_double(area)) < 0.001);
    
    // Test 5: Single point (area = 0)
    std::vector<Point_2> single = {Point_2(1,1)};
    area = polygon_area(single);
    assert(std::abs(CGAL::to_double(area)) < 0.001);
    
    // Test 6: Pentagon (catches out-of-bounds bugs)
    std::vector<Point_2> pentagon = {
        Point_2(0,0), Point_2(2,0), Point_2(3,2), Point_2(1,3), Point_2(-1,2)
    };
    area = polygon_area(pentagon);
    assert(std::abs(CGAL::to_double(area) - 8.0) < 0.001);
    
    std::cout << "test_polygon_area: PASSED\n";
}

int main() {
    test_polygon_area();
    return 0;
}
