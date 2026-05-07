#include "../src/cgal_kernel_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    using namespace CGALKernel;
    
    const double TOLERANCE = 1e-9;
    
    // ========================================
    // HELPER: Check if two points are approximately equal
    // ========================================
    auto points_equal = [TOLERANCE](const Point_2& p1, const Point_2& p2) {
        double dx = CGAL::to_double(p1.x() - p2.x());
        double dy = CGAL::to_double(p1.y() - p2.y());
        return std::abs(dx) < TOLERANCE && std::abs(dy) < TOLERANCE;
    };
    
    // ========================================
    // PROPERTY 1: Midpoint is equidistant from both endpoints
    // ========================================
    auto check_equidistant = [&](const Point_2& p, const Point_2& q) {
        Point_2 mid = midpoint(p, q);
        FT dist_p = squared_distance(p, mid);
        FT dist_q = squared_distance(q, mid);
        
        double d_p = CGAL::to_double(dist_p);
        double d_q = CGAL::to_double(dist_q);
        
        assert(std::abs(d_p - d_q) < TOLERANCE && 
               "Midpoint must be equidistant from both endpoints");
    };
    
    // ========================================
    // PROPERTY 2: Midpoint is collinear with endpoints
    // ========================================
    auto check_collinear_property = [](const Point_2& p, const Point_2& q) {
        Point_2 mid = midpoint(p, q);
        assert(collinear(p, mid, q) && 
               "Midpoint must be collinear with both endpoints");
    };
    
    // ========================================
    // PROPERTY 3: Symmetry - midpoint(p,q) == midpoint(q,p)
    // ========================================
    auto check_symmetry = [&](const Point_2& p, const Point_2& q) {
        Point_2 mid_pq = midpoint(p, q);
        Point_2 mid_qp = midpoint(q, p);
        
        assert(points_equal(mid_pq, mid_qp) && 
               "Midpoint must be symmetric: midpoint(p,q) == midpoint(q,p)");
    };
    
    // ========================================
    // PROPERTY 4: Midpoint coordinates are average of endpoint coordinates
    // ========================================
    auto check_coordinate_average = [&](const Point_2& p, const Point_2& q) {
        Point_2 mid = midpoint(p, q);
        
        double expected_x = (CGAL::to_double(p.x()) + CGAL::to_double(q.x())) / 2.0;
        double expected_y = (CGAL::to_double(p.y()) + CGAL::to_double(q.y())) / 2.0;
        
        double actual_x = CGAL::to_double(mid.x());
        double actual_y = CGAL::to_double(mid.y());
        
        assert(std::abs(actual_x - expected_x) < TOLERANCE && 
               "Midpoint x-coordinate must be average of endpoint x-coordinates");
        assert(std::abs(actual_y - expected_y) < TOLERANCE && 
               "Midpoint y-coordinate must be average of endpoint y-coordinates");
    };
    
    // ========================================
    // PROPERTY 5: Degenerate case - midpoint of identical points is the point itself
    // ========================================
    auto check_degenerate = [&](const Point_2& p) {
        Point_2 mid = midpoint(p, p);
        assert(points_equal(mid, p) && 
               "Midpoint of identical points must be the point itself");
    };
    
    // ========================================
    // PROPERTY 6: Distance property - distance from p to mid + distance from mid to q = distance from p to q
    // ========================================
    auto check_distance_sum = [&](const Point_2& p, const Point_2& q) {
        Point_2 mid = midpoint(p, q);
        
        double dist_p_mid = std::sqrt(CGAL::to_double(squared_distance(p, mid)));
        double dist_mid_q = std::sqrt(CGAL::to_double(squared_distance(mid, q)));
        double dist_p_q = std::sqrt(CGAL::to_double(squared_distance(p, q)));
        
        double sum = dist_p_mid + dist_mid_q;
        
        assert(std::abs(sum - dist_p_q) < TOLERANCE && 
               "Sum of distances p->mid and mid->q must equal distance p->q");
    };
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: Horizontal segment
    Point_2 p1(0, 0), q1(2, 0);
    Point_2 mid1 = midpoint(p1, q1);
    assert(points_equal(mid1, Point_2(1, 0)) && 
           "Midpoint of horizontal segment should be correct");
    check_equidistant(p1, q1);
    check_collinear_property(p1, q1);
    check_symmetry(p1, q1);
    check_coordinate_average(p1, q1);
    check_distance_sum(p1, q1);
    
    // Test 2: Vertical segment
    Point_2 p2(0, 0), q2(0, 4);
    Point_2 mid2 = midpoint(p2, q2);
    assert(points_equal(mid2, Point_2(0, 2)) && 
           "Midpoint of vertical segment should be correct");
    check_equidistant(p2, q2);
    check_collinear_property(p2, q2);
    check_symmetry(p2, q2);
    check_coordinate_average(p2, q2);
    check_distance_sum(p2, q2);
    
    // Test 3: Diagonal segment
    Point_2 p3(1, 1), q3(3, 3);
    Point_2 mid3 = midpoint(p3, q3);
    assert(points_equal(mid3, Point_2(2, 2)) && 
           "Midpoint of diagonal segment should be correct");
    check_equidistant(p3, q3);
    check_collinear_property(p3, q3);
    check_symmetry(p3, q3);
    check_coordinate_average(p3, q3);
    check_distance_sum(p3, q3);
    
    // Test 4: Negative coordinates
    Point_2 p4(-1, -1), q4(1, 1);
    Point_2 mid4 = midpoint(p4, q4);
    assert(points_equal(mid4, Point_2(0, 0)) && 
           "Midpoint with negative coordinates should be correct");
    check_equidistant(p4, q4);
    check_collinear_property(p4, q4);
    check_symmetry(p4, q4);
    check_coordinate_average(p4, q4);
    check_distance_sum(p4, q4);
    
    // Test 5: Large coordinates
    Point_2 p5(0, 0), q5(1000, 1000);
    check_equidistant(p5, q5);
    check_collinear_property(p5, q5);
    check_symmetry(p5, q5);
    check_coordinate_average(p5, q5);
    check_distance_sum(p5, q5);
    
    // Test 6: Degenerate case - same point
    Point_2 p6(5, 5);
    check_degenerate(p6);
    
    // Test 7: Mixed positive/negative
    Point_2 p7(-5, 3), q7(5, -3);
    Point_2 mid7 = midpoint(p7, q7);
    assert(points_equal(mid7, Point_2(0, 0)) && 
           "Midpoint of mixed coordinates should be correct");
    check_equidistant(p7, q7);
    check_collinear_property(p7, q7);
    check_symmetry(p7, q7);
    check_coordinate_average(p7, q7);
    
    // Test 8: Fractional coordinates
    Point_2 p8(0.5, 0.5), q8(1.5, 1.5);
    Point_2 mid8 = midpoint(p8, q8);
    assert(points_equal(mid8, Point_2(1.0, 1.0)) && 
           "Midpoint of fractional coordinates should be correct");
    check_equidistant(p8, q8);
    check_symmetry(p8, q8);
    check_coordinate_average(p8, q8);
    
    // Test 9: Asymmetric segment
    Point_2 p9(1, 2), q9(5, 8);
    check_equidistant(p9, q9);
    check_collinear_property(p9, q9);
    check_symmetry(p9, q9);
    check_coordinate_average(p9, q9);
    check_distance_sum(p9, q9);
    
    // Test 10: Very close points
    Point_2 p10(0, 0), q10(0.001, 0.001);
    check_equidistant(p10, q10);
    check_collinear_property(p10, q10);
    check_symmetry(p10, q10);
    check_coordinate_average(p10, q10);
    
    std::cout << "test_midpoint_strict: PASSED (10 test cases with property validation)\n";
    return 0;
}
