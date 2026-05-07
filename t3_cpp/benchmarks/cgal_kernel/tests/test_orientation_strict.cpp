#include "../src/cgal_kernel_functions.h"
#include <cassert>
#include <iostream>

int main() {
    using namespace CGALKernel;
    
    // ========================================
    // PROPERTY 1: Consistency with left_turn and right_turn
    // ========================================
    auto check_turn_consistency = [](const Point_2& p, const Point_2& q, const Point_2& r) {
        CGAL::Orientation orient = orientation(p, q, r);
        bool is_left = left_turn(p, q, r);
        bool is_right = right_turn(p, q, r);
        bool is_collinear_result = collinear(p, q, r);
        
        if (orient == CGAL::LEFT_TURN) {
            assert(is_left && "LEFT_TURN orientation must match left_turn() == true");
            assert(!is_right && "LEFT_TURN orientation must match right_turn() == false");
        }
        if (orient == CGAL::RIGHT_TURN) {
            assert(is_right && "RIGHT_TURN orientation must match right_turn() == true");
            assert(!is_left && "RIGHT_TURN orientation must match left_turn() == false");
        }
        if (orient == CGAL::COLLINEAR) {
            assert(!is_left && "COLLINEAR orientation must match left_turn() == false");
            assert(!is_right && "COLLINEAR orientation must match right_turn() == false");
            assert(is_collinear_result && "COLLINEAR orientation must match collinear() == true");
        }
    };
    
    // ========================================
    // PROPERTY 2: Antisymmetry - swapping last two points reverses orientation
    // ========================================
    auto check_antisymmetry = [](const Point_2& p, const Point_2& q, const Point_2& r) {
        CGAL::Orientation orient_pqr = orientation(p, q, r);
        CGAL::Orientation orient_prq = orientation(p, r, q);
        
        if (orient_pqr == CGAL::LEFT_TURN) {
            assert(orient_prq == CGAL::RIGHT_TURN && 
                   "Swapping last two points should reverse LEFT_TURN to RIGHT_TURN");
        } else if (orient_pqr == CGAL::RIGHT_TURN) {
            assert(orient_prq == CGAL::LEFT_TURN && 
                   "Swapping last two points should reverse RIGHT_TURN to LEFT_TURN");
        } else {
            assert(orient_prq == CGAL::COLLINEAR && 
                   "COLLINEAR should remain COLLINEAR after swapping");
        }
    };
    
    // ========================================
    // PROPERTY 3: Degenerate cases - identical points are collinear
    // ========================================
    auto check_degenerate = [](const Point_2& p, const Point_2& q) {
        assert(orientation(p, p, p) == CGAL::COLLINEAR && 
               "Three identical points must be COLLINEAR");
        assert(orientation(p, p, q) == CGAL::COLLINEAR && 
               "Two identical points must be COLLINEAR");
        assert(orientation(p, q, p) == CGAL::COLLINEAR && 
               "Two identical points must be COLLINEAR");
        assert(orientation(q, p, p) == CGAL::COLLINEAR && 
               "Two identical points must be COLLINEAR");
    };
    
    // ========================================
    // PROPERTY 4: Cyclic permutation consistency
    // ========================================
    auto check_cyclic = [](const Point_2& p, const Point_2& q, const Point_2& r) {
        CGAL::Orientation orient_pqr = orientation(p, q, r);
        CGAL::Orientation orient_qrp = orientation(q, r, p);
        CGAL::Orientation orient_rpq = orientation(r, p, q);
        
        // Cyclic permutations should preserve orientation
        assert(orient_pqr == orient_qrp && 
               "Cyclic permutation (p,q,r) -> (q,r,p) should preserve orientation");
        assert(orient_pqr == orient_rpq && 
               "Cyclic permutation (p,q,r) -> (r,p,q) should preserve orientation");
    };
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: Clear left turn (counterclockwise)
    Point_2 p1(0, 0), q1(1, 0), r1(1, 1);
    assert(orientation(p1, q1, r1) == CGAL::LEFT_TURN && 
           "Moving from (0,0)->(1,0)->(1,1) should be LEFT_TURN");
    check_turn_consistency(p1, q1, r1);
    check_antisymmetry(p1, q1, r1);
    check_cyclic(p1, q1, r1);
    
    // Test 2: Clear right turn (clockwise)
    Point_2 p2(0, 0), q2(1, 0), r2(1, -1);
    assert(orientation(p2, q2, r2) == CGAL::RIGHT_TURN && 
           "Moving from (0,0)->(1,0)->(1,-1) should be RIGHT_TURN");
    check_turn_consistency(p2, q2, r2);
    check_antisymmetry(p2, q2, r2);
    check_cyclic(p2, q2, r2);
    
    // Test 3: Collinear points (straight line)
    Point_2 p3(0, 0), q3(1, 0), r3(2, 0);
    assert(orientation(p3, q3, r3) == CGAL::COLLINEAR && 
           "Points on straight line should be COLLINEAR");
    check_turn_consistency(p3, q3, r3);
    check_antisymmetry(p3, q3, r3);
    check_cyclic(p3, q3, r3);
    
    // Test 4: Left turn with negative coordinates
    Point_2 p4(-1, -1), q4(0, -1), r4(0, 0);
    assert(orientation(p4, q4, r4) == CGAL::LEFT_TURN && 
           "LEFT_TURN should work with negative coordinates");
    check_turn_consistency(p4, q4, r4);
    check_antisymmetry(p4, q4, r4);
    
    // Test 5: Right turn with negative coordinates
    Point_2 p5(-1, 1), q5(0, 1), r5(0, 0);
    assert(orientation(p5, q5, r5) == CGAL::RIGHT_TURN && 
           "RIGHT_TURN should work with negative coordinates");
    check_turn_consistency(p5, q5, r5);
    check_antisymmetry(p5, q5, r5);
    
    // Test 6: Vertical collinear
    Point_2 p6(0, 0), q6(0, 1), r6(0, 2);
    assert(orientation(p6, q6, r6) == CGAL::COLLINEAR && 
           "Vertical line should be COLLINEAR");
    check_turn_consistency(p6, q6, r6);
    check_antisymmetry(p6, q6, r6);
    
    // Test 7: Diagonal collinear
    Point_2 p7(0, 0), q7(1, 1), r7(2, 2);
    assert(orientation(p7, q7, r7) == CGAL::COLLINEAR && 
           "Diagonal line should be COLLINEAR");
    check_turn_consistency(p7, q7, r7);
    check_antisymmetry(p7, q7, r7);
    
    // Test 8: Degenerate cases
    Point_2 p8(5, 5), q8(10, 10);
    check_degenerate(p8, q8);
    
    // Test 9: Large coordinates left turn
    Point_2 p9(0, 0), q9(1000, 0), r9(1000, 1000);
    assert(orientation(p9, q9, r9) == CGAL::LEFT_TURN && 
           "LEFT_TURN should work with large coordinates");
    check_turn_consistency(p9, q9, r9);
    
    // Test 10: Large coordinates right turn
    Point_2 p10(0, 0), q10(1000, 0), r10(1000, -1000);
    assert(orientation(p10, q10, r10) == CGAL::RIGHT_TURN && 
           "RIGHT_TURN should work with large coordinates");
    check_turn_consistency(p10, q10, r10);
    
    // Test 11: Sharp left turn (small angle)
    Point_2 p11(0, 0), q11(10, 0), r11(10, 1);
    assert(orientation(p11, q11, r11) == CGAL::LEFT_TURN && 
           "Small angle LEFT_TURN should be detected");
    check_turn_consistency(p11, q11, r11);
    
    // Test 12: Sharp right turn (small angle)
    Point_2 p12(0, 0), q12(10, 0), r12(10, -1);
    assert(orientation(p12, q12, r12) == CGAL::RIGHT_TURN && 
           "Small angle RIGHT_TURN should be detected");
    check_turn_consistency(p12, q12, r12);
    
    std::cout << "test_orientation_strict: PASSED (12 test cases with property validation)\n";
    return 0;
}
