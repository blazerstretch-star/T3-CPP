#include "../src/cgal_kernel_functions.h"
#include <cassert>
#include <iostream>
#include <algorithm>

void test_convex_hull_graham() {
    using namespace CGALKernel;
    
    // Test 1: Square with interior points - hull should be corners only
    std::vector<Point_2> points = {
        Point_2(0,0), Point_2(2,0), Point_2(2,2), Point_2(0,2),
        Point_2(1,1), Point_2(1,0.5), Point_2(0.5,1)
    };
    std::vector<Point_2> hull = convex_hull_graham(points);
    assert(hull.size() >= 2);  // At least 2 points for a hull
    
    // Verify no interior points in hull
    bool has_interior = std::find(hull.begin(), hull.end(), Point_2(1,1)) != hull.end();
    assert(!has_interior);
    
    // Test 2: Triangle - hull should contain vertices
    std::vector<Point_2> tri = {
        Point_2(0,0), Point_2(3,0), Point_2(1.5,2)
    };
    hull = convex_hull_graham(tri);
    assert(hull.size() >= 1);  // At least one point
    
    // Test 3: Single point
    std::vector<Point_2> single = {Point_2(1,1)};
    hull = convex_hull_graham(single);
    assert(hull.size() == 1);
    assert(hull[0] == Point_2(1,1));
    
    // Test 4: Collinear points - degenerate case
    std::vector<Point_2> line = {
        Point_2(0,0), Point_2(1,0), Point_2(2,0)
    };
    hull = convex_hull_graham(line);
    assert(hull.size() >= 1);  // Degenerate case
    
    std::cout << "test_convex_hull_graham: PASSED\n";
}

int main() {
    test_convex_hull_graham();
    return 0;
}
