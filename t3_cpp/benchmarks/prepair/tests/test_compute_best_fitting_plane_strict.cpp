#include "../src/cgal_functions.h"
#include <cassert>
#include <cmath>

int main() {
    const double DISTANCE_TOLERANCE = 0.1;  // Points should be close to plane
    const double NORMAL_MIN_LENGTH = 0.01;  // Plane must have non-degenerate normal
    
    // ========================================
    // PROPERTY 1: All input points lie on the fitted plane
    // Mathematical invariant: distance(point, plane) ≈ 0
    // ========================================
    auto check_points_on_plane = [DISTANCE_TOLERANCE](
        const std::list<Kernel::Point_3>& points,
        const Kernel::Plane_3& plane) {
        
        for (const auto& p : points) {
            // Calculate signed distance: ax + by + cz + d
            double dist = std::abs(CGAL::to_double(
                plane.a() * p.x() + 
                plane.b() * p.y() + 
                plane.c() * p.z() + 
                plane.d()
            ));
            
            assert(dist < DISTANCE_TOLERANCE &&
                   "PROPERTY VIOLATION: All points must lie on fitted plane");
        }
    };
    
    // ========================================
    // PROPERTY 2: Plane has non-degenerate normal
    // The normal vector must have non-zero length
    // ========================================
    auto check_non_degenerate_normal = [NORMAL_MIN_LENGTH](const Kernel::Plane_3& plane) {
        Kernel::Vector_3 normal = plane.orthogonal_vector();
        double normal_length_sq = CGAL::to_double(normal.squared_length());
        
        assert(normal_length_sq > NORMAL_MIN_LENGTH &&
               "PROPERTY VIOLATION: Plane must have non-degenerate normal");
    };
    
    // ========================================
    // PROPERTY 3: Plane is unique for coplanar points
    // If all points are coplanar, any valid plane should contain them
    // ========================================
    auto check_plane_uniqueness = [DISTANCE_TOLERANCE](
        const std::list<Kernel::Point_3>& points,
        const Kernel::Plane_3& plane1,
        const Kernel::Plane_3& plane2) {
        
        // Both planes should contain all points
        for (const auto& p : points) {
            double dist1 = std::abs(CGAL::to_double(
                plane1.a() * p.x() + plane1.b() * p.y() + plane1.c() * p.z() + plane1.d()
            ));
            double dist2 = std::abs(CGAL::to_double(
                plane2.a() * p.x() + plane2.b() * p.y() + plane2.c() * p.z() + plane2.d()
            ));
            
            assert(dist1 < DISTANCE_TOLERANCE && dist2 < DISTANCE_TOLERANCE &&
                   "PROPERTY VIOLATION: Both planes must contain all points");
        }
    };
    
    // ========================================
    // PROPERTY 4: Deterministic output
    // Same input should produce same plane (or equivalent plane)
    // ========================================
    auto check_determinism = [DISTANCE_TOLERANCE](
        const std::list<Kernel::Point_3>& points) {
        
        Kernel::Plane_3 plane1 = CGALFunctions::compute_best_fitting_plane(points);
        Kernel::Plane_3 plane2 = CGALFunctions::compute_best_fitting_plane(points);
        
        // Both planes should contain all points (they might differ by sign/scale)
        for (const auto& p : points) {
            double dist1 = std::abs(CGAL::to_double(
                plane1.a() * p.x() + plane1.b() * p.y() + plane1.c() * p.z() + plane1.d()
            ));
            double dist2 = std::abs(CGAL::to_double(
                plane2.a() * p.x() + plane2.b() * p.y() + plane2.c() * p.z() + plane2.d()
            ));
            
            assert(dist1 < DISTANCE_TOLERANCE && dist2 < DISTANCE_TOLERANCE &&
                   "PROPERTY VIOLATION: Function must be deterministic");
        }
    };
    
    // ========================================
    // TEST 1: Points on XY plane (z=0)
    // ========================================
    {
        std::list<Kernel::Point_3> points;
        points.push_back(Kernel::Point_3(0, 0, 0));
        points.push_back(Kernel::Point_3(1, 0, 0));
        points.push_back(Kernel::Point_3(0, 1, 0));
        points.push_back(Kernel::Point_3(1, 1, 0));
        
        Kernel::Plane_3 plane = CGALFunctions::compute_best_fitting_plane(points);
        
        check_points_on_plane(points, plane);
        check_non_degenerate_normal(plane);
        check_determinism(points);
    }
    
    // ========================================
    // TEST 2: Points on tilted plane
    // ========================================
    {
        std::list<Kernel::Point_3> points;
        points.push_back(Kernel::Point_3(0, 0, 0));
        points.push_back(Kernel::Point_3(1, 0, 1));
        points.push_back(Kernel::Point_3(0, 1, 1));
        points.push_back(Kernel::Point_3(1, 1, 2));
        
        Kernel::Plane_3 plane = CGALFunctions::compute_best_fitting_plane(points);
        
        check_points_on_plane(points, plane);
        check_non_degenerate_normal(plane);
        check_determinism(points);
    }
    
    // ========================================
    // TEST 3: Points on vertical plane (x=0)
    // ========================================
    {
        std::list<Kernel::Point_3> points;
        points.push_back(Kernel::Point_3(0, 0, 0));
        points.push_back(Kernel::Point_3(0, 1, 0));
        points.push_back(Kernel::Point_3(0, 0, 1));
        points.push_back(Kernel::Point_3(0, 1, 1));
        
        Kernel::Plane_3 plane = CGALFunctions::compute_best_fitting_plane(points);
        
        check_points_on_plane(points, plane);
        check_non_degenerate_normal(plane);
        check_determinism(points);
    }
    
    // ========================================
    // TEST 4: Exactly 3 points (minimal case)
    // ========================================
    {
        std::list<Kernel::Point_3> points;
        points.push_back(Kernel::Point_3(0, 0, 0));
        points.push_back(Kernel::Point_3(1, 0, 0));
        points.push_back(Kernel::Point_3(0, 1, 0));
        
        Kernel::Plane_3 plane = CGALFunctions::compute_best_fitting_plane(points);
        
        check_points_on_plane(points, plane);
        check_non_degenerate_normal(plane);
        check_determinism(points);
    }
    
    // ========================================
    // TEST 5: Less than 3 points (degenerate case)
    // Should return a valid plane without crashing
    // ========================================
    {
        std::list<Kernel::Point_3> points;
        points.push_back(Kernel::Point_3(0, 0, 0));
        points.push_back(Kernel::Point_3(1, 0, 0));
        
        Kernel::Plane_3 plane = CGALFunctions::compute_best_fitting_plane(points);
        
        // Must return a valid plane (non-degenerate normal)
        check_non_degenerate_normal(plane);
        
        // Points should be reasonably close to the plane
        // (but we allow larger tolerance for degenerate case)
        for (const auto& p : points) {
            double dist = std::abs(CGAL::to_double(
                plane.a() * p.x() + plane.b() * p.y() + plane.c() * p.z() + plane.d()
            ));
            assert(dist < 1.0 &&
                   "PROPERTY VIOLATION: Points should be reasonably close to plane");
        }
    }
    
    // ========================================
    // TEST 6: Single point (extreme degenerate case)
    // ========================================
    {
        std::list<Kernel::Point_3> points;
        points.push_back(Kernel::Point_3(5, 5, 5));
        
        Kernel::Plane_3 plane = CGALFunctions::compute_best_fitting_plane(points);
        
        // Must return a valid plane
        check_non_degenerate_normal(plane);
    }
    
    // ========================================
    // TEST 7: Empty input (extreme edge case)
    // ========================================
    {
        std::list<Kernel::Point_3> points;
        
        Kernel::Plane_3 plane = CGALFunctions::compute_best_fitting_plane(points);
        
        // Must return a valid plane without crashing
        check_non_degenerate_normal(plane);
    }
    
    // ========================================
    // TEST 8: Nearly colinear points (numerical stability)
    // ========================================
    {
        std::list<Kernel::Point_3> points;
        points.push_back(Kernel::Point_3(0, 0, 0));
        points.push_back(Kernel::Point_3(1, 0, 0.001));
        points.push_back(Kernel::Point_3(2, 0, 0.002));
        points.push_back(Kernel::Point_3(3, 0, 0.003));
        
        Kernel::Plane_3 plane = CGALFunctions::compute_best_fitting_plane(points);
        
        check_points_on_plane(points, plane);
        check_non_degenerate_normal(plane);
    }
    
    // ========================================
    // TEST 9: Points with large coordinates
    // ========================================
    {
        std::list<Kernel::Point_3> points;
        points.push_back(Kernel::Point_3(1000, 2000, 3000));
        points.push_back(Kernel::Point_3(1001, 2000, 3000));
        points.push_back(Kernel::Point_3(1000, 2001, 3000));
        points.push_back(Kernel::Point_3(1001, 2001, 3000));
        
        Kernel::Plane_3 plane = CGALFunctions::compute_best_fitting_plane(points);
        
        check_points_on_plane(points, plane);
        check_non_degenerate_normal(plane);
    }
    
    // ========================================
    // TEST 10: Points with negative coordinates
    // ========================================
    {
        std::list<Kernel::Point_3> points;
        points.push_back(Kernel::Point_3(-1, -1, -1));
        points.push_back(Kernel::Point_3(-2, -1, -1));
        points.push_back(Kernel::Point_3(-1, -2, -1));
        points.push_back(Kernel::Point_3(-2, -2, -1));
        
        Kernel::Plane_3 plane = CGALFunctions::compute_best_fitting_plane(points);
        
        check_points_on_plane(points, plane);
        check_non_degenerate_normal(plane);
    }
    
    // ========================================
    // TEST 11: Many points (stress test)
    // ========================================
    {
        std::list<Kernel::Point_3> points;
        // Generate 100 points on plane z = x + y
        for (int i = 0; i < 10; ++i) {
            for (int j = 0; j < 10; ++j) {
                double x = i * 0.1;
                double y = j * 0.1;
                double z = x + y;
                points.push_back(Kernel::Point_3(x, y, z));
            }
        }
        
        Kernel::Plane_3 plane = CGALFunctions::compute_best_fitting_plane(points);
        
        check_points_on_plane(points, plane);
        check_non_degenerate_normal(plane);
    }
    
    // ========================================
    // TEST 12: Points with noise (least squares property)
    // ========================================
    {
        std::list<Kernel::Point_3> points;
        // Points near z=0 plane with small noise
        points.push_back(Kernel::Point_3(0, 0, 0.01));
        points.push_back(Kernel::Point_3(1, 0, -0.01));
        points.push_back(Kernel::Point_3(0, 1, 0.02));
        points.push_back(Kernel::Point_3(1, 1, -0.02));
        
        Kernel::Plane_3 plane = CGALFunctions::compute_best_fitting_plane(points);
        
        // With noise, points should be close but not exactly on plane
        for (const auto& p : points) {
            double dist = std::abs(CGAL::to_double(
                plane.a() * p.x() + plane.b() * p.y() + plane.c() * p.z() + plane.d()
            ));
            assert(dist < 0.5 &&  // Larger tolerance for noisy data
                   "PROPERTY VIOLATION: Points should be reasonably close to fitted plane");
        }
        check_non_degenerate_normal(plane);
    }
    
    return 0;
}
