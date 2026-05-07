#include "../src/cgal_functions.h"
#include <cassert>
#include <cmath>

int main() {
    const double EPSILON = 1e-9;
    
    // ========================================
    // PROPERTY 1: Size preservation
    // Output list size must equal input vector size
    // ========================================
    auto check_size_preservation = [](const std::vector<std::vector<double>>& coords,
                                      const std::list<Kernel::Point_3>& points) {
        assert(points.size() == coords.size() && 
               "PROPERTY VIOLATION: Output size must equal input size");
    };
    
    // ========================================
    // PROPERTY 2: Coordinate accuracy
    // Each coordinate component must be preserved within tolerance
    // ========================================
    auto check_coordinate_accuracy = [EPSILON](const std::vector<std::vector<double>>& coords,
                                               const std::list<Kernel::Point_3>& points) {
        auto it = points.begin();
        for (size_t i = 0; i < coords.size(); ++i, ++it) {
            double expected_x = coords[i][0];
            double expected_y = coords[i][1];
            double expected_z = (coords[i].size() >= 3) ? coords[i][2] : 0.0;
            
            double actual_x = CGAL::to_double(it->x());
            double actual_y = CGAL::to_double(it->y());
            double actual_z = CGAL::to_double(it->z());
            
            assert(std::abs(actual_x - expected_x) < EPSILON &&
                   "PROPERTY VIOLATION: X coordinate not preserved");
            assert(std::abs(actual_y - expected_y) < EPSILON &&
                   "PROPERTY VIOLATION: Y coordinate not preserved");
            assert(std::abs(actual_z - expected_z) < EPSILON &&
                   "PROPERTY VIOLATION: Z coordinate not preserved");
        }
    };
    
    // ========================================
    // PROPERTY 3: Order preservation
    // Points must appear in same order as input
    // ========================================
    auto check_order_preservation = [EPSILON](const std::vector<std::vector<double>>& coords,
                                              const std::list<Kernel::Point_3>& points) {
        auto it = points.begin();
        for (size_t i = 0; i < coords.size(); ++i, ++it) {
            // Verify this is the i-th point by checking coordinates
            double expected_x = coords[i][0];
            double actual_x = CGAL::to_double(it->x());
            assert(std::abs(actual_x - expected_x) < EPSILON &&
                   "PROPERTY VIOLATION: Point order not preserved");
        }
    };
    
    // ========================================
    // TEST 1: 3D coordinates (standard case)
    // ========================================
    {
        std::vector<std::vector<double>> coords = {{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}};
        std::list<Kernel::Point_3> points;
        CGALFunctions::extract_points_from_coordinates(coords, points);
        
        check_size_preservation(coords, points);
        check_coordinate_accuracy(coords, points);
        check_order_preservation(coords, points);
    }
    
    // ========================================
    // TEST 2: 2D coordinates (z defaults to 0)
    // ========================================
    {
        std::vector<std::vector<double>> coords = {{1.0, 2.0}, {3.0, 4.0}};
        std::list<Kernel::Point_3> points;
        CGALFunctions::extract_points_from_coordinates(coords, points);
        
        check_size_preservation(coords, points);
        check_coordinate_accuracy(coords, points);
        
        // Additional check: z must be 0 for 2D input
        for (const auto& p : points) {
            assert(std::abs(CGAL::to_double(p.z()) - 0.0) < EPSILON &&
                   "PROPERTY VIOLATION: 2D coordinates must have z=0");
        }
    }
    
    // ========================================
    // TEST 3: Mixed 2D and 3D coordinates
    // ========================================
    {
        std::vector<std::vector<double>> coords = {
            {1.0, 2.0},        // 2D
            {3.0, 4.0, 5.0},   // 3D
            {6.0, 7.0}         // 2D
        };
        std::list<Kernel::Point_3> points;
        CGALFunctions::extract_points_from_coordinates(coords, points);
        
        check_size_preservation(coords, points);
        check_coordinate_accuracy(coords, points);
        check_order_preservation(coords, points);
    }
    
    // ========================================
    // TEST 4: Empty input
    // ========================================
    {
        std::vector<std::vector<double>> coords = {};
        std::list<Kernel::Point_3> points;
        CGALFunctions::extract_points_from_coordinates(coords, points);
        
        assert(points.empty() && 
               "PROPERTY VIOLATION: Empty input must produce empty output");
    }
    
    // ========================================
    // TEST 5: Single point
    // ========================================
    {
        std::vector<std::vector<double>> coords = {{1.5, 2.5, 3.5}};
        std::list<Kernel::Point_3> points;
        CGALFunctions::extract_points_from_coordinates(coords, points);
        
        check_size_preservation(coords, points);
        check_coordinate_accuracy(coords, points);
    }
    
    // ========================================
    // TEST 6: Large coordinates (numerical stability)
    // ========================================
    {
        std::vector<std::vector<double>> coords = {
            {1000000.0, 2000000.0, 3000000.0},
            {-1000000.0, -2000000.0, -3000000.0}
        };
        std::list<Kernel::Point_3> points;
        CGALFunctions::extract_points_from_coordinates(coords, points);
        
        check_size_preservation(coords, points);
        check_coordinate_accuracy(coords, points);
    }
    
    // ========================================
    // TEST 7: Negative coordinates
    // ========================================
    {
        std::vector<std::vector<double>> coords = {
            {-1.0, -2.0, -3.0},
            {-4.0, -5.0}
        };
        std::list<Kernel::Point_3> points;
        CGALFunctions::extract_points_from_coordinates(coords, points);
        
        check_size_preservation(coords, points);
        check_coordinate_accuracy(coords, points);
    }
    
    // ========================================
    // TEST 8: Zero coordinates
    // ========================================
    {
        std::vector<std::vector<double>> coords = {
            {0.0, 0.0, 0.0},
            {0.0, 0.0}
        };
        std::list<Kernel::Point_3> points;
        CGALFunctions::extract_points_from_coordinates(coords, points);
        
        check_size_preservation(coords, points);
        check_coordinate_accuracy(coords, points);
    }
    
    // ========================================
    // TEST 9: Fractional coordinates (precision test)
    // ========================================
    {
        std::vector<std::vector<double>> coords = {
            {1.123456789, 2.987654321, 3.555555555},
            {0.000000001, 0.999999999}
        };
        std::list<Kernel::Point_3> points;
        CGALFunctions::extract_points_from_coordinates(coords, points);
        
        check_size_preservation(coords, points);
        check_coordinate_accuracy(coords, points);
    }
    
    // ========================================
    // TEST 10: Many points (stress test)
    // ========================================
    {
        std::vector<std::vector<double>> coords;
        for (int i = 0; i < 1000; ++i) {
            coords.push_back({(double)i, (double)(i*2), (double)(i*3)});
        }
        std::list<Kernel::Point_3> points;
        CGALFunctions::extract_points_from_coordinates(coords, points);
        
        check_size_preservation(coords, points);
        check_coordinate_accuracy(coords, points);
        check_order_preservation(coords, points);
    }
    
    return 0;
}
