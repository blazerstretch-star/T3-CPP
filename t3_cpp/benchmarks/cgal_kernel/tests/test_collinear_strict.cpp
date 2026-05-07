#include "../src/cgal_kernel_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    using namespace CGALKernel;
    
    // ========================================
    // PROPERTY 1: Symmetry - order shouldn't matter for collinearity
    // ========================================
    auto check_symmetry = [](const Point_2& p, const Point_2& q, const Point_2& r) {
        bool result_pqr = collinear(p, q, r);
        bool result_prq = collinear(p, r, q);
        bool result_qpr = collinear(q, p, r);
        bool result_qrp = collinear(q, r, p);
        bool result_rpq = collinear(r, p, q);
        bool result_rqp = collinear(r, q, p);
        
        assert(result_pqr == result_prq && "Collinearity must be symmetric (p,q,r) == (p,r,q)");
        assert(result_pqr == result_qpr && "Collinearity must be symmetric (p,q,r) == (q,p,r)");
        assert(result_pqr == result_qrp && "Collinearity must be symmetric (p,q,r) == (q,r,p)");
        assert(result_pqr == result_rpq && "Collinearity must be symmetric (p,q,r) == (r,p,q)");
        assert(result_pqr == result_rqp && "Collinearity must be symmetric (p,q,r) == (r,q,p)");
    };
    
    // ========================================
    // PROPERTY 2: Degenerate cases - identical points are always collinear
    // ========================================
    auto check_degenerate = [](const Point_2& p, const Point_2& q) {
        assert(collinear(p, p, p) && "Three identical points must be collinear");
        assert(collinear(p, p, q) && "Two identical points must be collinear with any third");
        assert(collinear(p, q, p) && "Two identical points must be collinear with any third");
        assert(collinear(q, p, p) && "Two identical points must be collinear with any third");
    };
    
    // ========================================
    // PROPERTY 3: Consistency with orientation
    // ========================================
    auto check_orientation_consistency = [](const Point_2& p, const Point_2& q, const Point_2& r) {
        bool is_collinear = collinear(p, q, r);
        CGAL::Orientation orient = orientation(p, q, r);
        
        if (is_collinear) {
            assert(orient == CGAL::COLLINEAR && 
                   "If collinear returns true, orientation must be COLLINEAR");
        }
        if (orient == CGAL::COLLINEAR) {
            assert(is_collinear && 
                   "If orientation is COLLINEAR, collinear must return true");
        }
    };
    
    // ========================================
    // PROPERTY 4: Transitivity (if p,q,r collinear and q,r,s collinear, check consistency)
    // ========================================
    auto check_line_consistency = [](const Point_2& p, const Point_2& q, 
                                     const Point_2& r, const Point_2& s) {
        if (collinear(p, q, r) && collinear(q, r, s)) {
            // All four points should be collinear
            assert(collinear(p, q, s) && "Transitivity: if p,q,r and q,r,s collinear, then p,q,s collinear");
            assert(collinear(p, r, s) && "Transitivity: if p,q,r and q,r,s collinear, then p,r,s collinear");
        }
    };
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: Diagonal line (positive slope)
    Point_2 p1(0, 0), q1(1, 1), r1(2, 2);
    assert(collinear(p1, q1, r1) && "Points on diagonal line y=x should be collinear");
    check_symmetry(p1, q1, r1);
    check_orientation_consistency(p1, q1, r1);
    
    // Test 2: Horizontal line
    Point_2 p2(0, 0), q2(1, 0), r2(2, 0);
    assert(collinear(p2, q2, r2) && "Points on horizontal line should be collinear");
    check_symmetry(p2, q2, r2);
    check_orientation_consistency(p2, q2, r2);
    
    // Test 3: Vertical line
    Point_2 p3(0, 0), q3(0, 1), r3(0, 2);
    assert(collinear(p3, q3, r3) && "Points on vertical line should be collinear");
    check_symmetry(p3, q3, r3);
    check_orientation_consistency(p3, q3, r3);
    
    // Test 4: Negative coordinates
    Point_2 p4(-2, -2), q4(0, 0), r4(2, 2);
    assert(collinear(p4, q4, r4) && "Collinearity should work with negative coordinates");
    check_symmetry(p4, q4, r4);
    check_orientation_consistency(p4, q4, r4);
    
    // Test 5: Large coordinates
    Point_2 p5(0, 0), q5(1000, 1000), r5(2000, 2000);
    assert(collinear(p5, q5, r5) && "Collinearity should work with large coordinates");
    check_symmetry(p5, q5, r5);
    check_orientation_consistency(p5, q5, r5);
    
    // Test 6: Not collinear (right angle)
    Point_2 p6(0, 0), q6(1, 0), r6(0, 1);
    assert(!collinear(p6, q6, r6) && "Points forming right angle should not be collinear");
    check_symmetry(p6, q6, r6);
    check_orientation_consistency(p6, q6, r6);
    
    // Test 7: All same point
    Point_2 p7(5, 5);
    check_degenerate(p7, p7);
    
    // Test 8: Two same points
    Point_2 p8(1, 1), q8(2, 2);
    check_degenerate(p8, q8);
    
    // Test 9: Negative slope line
    Point_2 p9(0, 2), q9(1, 1), r9(2, 0);
    assert(collinear(p9, q9, r9) && "Points on negative slope line should be collinear");
    check_symmetry(p9, q9, r9);
    check_orientation_consistency(p9, q9, r9);
    
    // Test 10: Transitivity check
    Point_2 t1(0, 0), t2(1, 1), t3(2, 2), t4(3, 3);
    check_line_consistency(t1, t2, t3, t4);
    
    // Test 11: Mixed positive/negative coordinates
    Point_2 p11(-1, -1), q11(0, 0), r11(1, 1);
    assert(collinear(p11, q11, r11) && "Collinearity across origin should work");
    check_symmetry(p11, q11, r11);
    
    // Test 12: Steep slope
    Point_2 p12(0, 0), q12(1, 10), r12(2, 20);
    assert(collinear(p12, q12, r12) && "Points on steep slope should be collinear");
    check_symmetry(p12, q12, r12);
    
    std::cout << "test_collinear_strict: PASSED (12 test cases with property validation)\n";
    return 0;
}
