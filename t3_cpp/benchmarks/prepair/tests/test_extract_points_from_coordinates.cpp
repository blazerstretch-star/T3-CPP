#include "../src/cgal_functions.h"
#include <cassert>
#include <cmath>

int main() {
    // Test 1: 3D coordinates
    std::vector<std::vector<double>> coords = {{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}};
    std::list<Kernel::Point_3> points;
    CGALFunctions::extract_points_from_coordinates(coords, points);
    
    assert(points.size() == 2);
    
    auto it = points.begin();
    assert(std::abs(it->x() - 1.0) < 1e-9);
    assert(std::abs(it->y() - 2.0) < 1e-9);
    assert(std::abs(it->z() - 3.0) < 1e-9);
    
    ++it;
    assert(std::abs(it->x() - 4.0) < 1e-9);
    assert(std::abs(it->y() - 5.0) < 1e-9);
    assert(std::abs(it->z() - 6.0) < 1e-9);
    
    // Test 2: 2D coordinates (should default z=0)
    coords = {{1.0, 2.0}, {3.0, 4.0}};
    points.clear();
    CGALFunctions::extract_points_from_coordinates(coords, points);
    
    assert(points.size() == 2);
    it = points.begin();
    assert(std::abs(it->z() - 0.0) < 1e-9);
    
    return 0;
}
