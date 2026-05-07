#include "../src/cgal_functions.h"
#include <cassert>
#include <cmath>

int main() {
    // Test 1: Points on XY plane (z=0) - should return plane with normal ~(0,0,1)
    std::list<Kernel::Point_3> points;
    points.push_back(Kernel::Point_3(0, 0, 0));
    points.push_back(Kernel::Point_3(1, 0, 0));
    points.push_back(Kernel::Point_3(0, 1, 0));
    points.push_back(Kernel::Point_3(1, 1, 0));
    
    Kernel::Plane_3 plane = CGALFunctions::compute_best_fitting_plane(points);
    
    // Verify all points are close to the plane
    for (const auto& p : points) {
        double dist = std::abs(CGAL::to_double(plane.a() * p.x() + 
                                                plane.b() * p.y() + 
                                                plane.c() * p.z() + 
                                                plane.d()));
        assert(dist < 0.01);  // All points should be on the plane
    }
    
    // Verify plane normal is not degenerate
    Kernel::Vector_3 normal = plane.orthogonal_vector();
    double len_sq = CGAL::to_double(normal.squared_length());
    assert(len_sq > 0.1);  // Non-zero normal
    
    // Test 2: Non-XY plane (tilted plane) - should NOT return default XY plane
    points.clear();
    points.push_back(Kernel::Point_3(0, 0, 0));
    points.push_back(Kernel::Point_3(1, 0, 1));
    points.push_back(Kernel::Point_3(0, 1, 1));
    points.push_back(Kernel::Point_3(1, 1, 2));
    
    plane = CGALFunctions::compute_best_fitting_plane(points);
    
    // Verify all points are close to the fitted plane
    for (const auto& p : points) {
        double dist = std::abs(CGAL::to_double(plane.a() * p.x() + 
                                                plane.b() * p.y() + 
                                                plane.c() * p.z() + 
                                                plane.d()));
        assert(dist < 0.5);  // Points should be reasonably close to fitted plane
    }
    
    // Test 3: Edge case - less than 3 points
    points.clear();
    points.push_back(Kernel::Point_3(0, 0, 0));
    points.push_back(Kernel::Point_3(1, 0, 0));
    
    plane = CGALFunctions::compute_best_fitting_plane(points);
    // Should return valid plane without crashing
    normal = plane.orthogonal_vector();
    len_sq = CGAL::to_double(normal.squared_length());
    assert(len_sq > 0);  // Must have non-zero normal
    
    return 0;
}
