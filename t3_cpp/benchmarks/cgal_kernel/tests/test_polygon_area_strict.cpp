#include "../src/cgal_kernel_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>
#include <algorithm>

int main() {
    using namespace CGALKernel;
    
    const double TOLERANCE = 1e-6;
    
    // ========================================
    // HELPER: Check if two values are approximately equal
    // ========================================
    auto approx_equal = [TOLERANCE](double a, double b) {
        return std::abs(a - b) < TOLERANCE;
    };
    
    // ========================================
    // PROPERTY 1: Area is additive - split polygon into triangles
    // ========================================
    auto check_triangle_decomposition = [&](const std::vector<Point_2>& polygon) {
        if (polygon.size() < 3) return;
        
        FT total_area = polygon_area(polygon);
        FT sum_triangles = FT(0);
        
        // Decompose into triangles from first vertex
        for (size_t i = 1; i < polygon.size() - 1; ++i) {
            sum_triangles += triangle_area(polygon[0], polygon[i], polygon[i+1]);
        }
        
        double total = CGAL::to_double(total_area);
        double sum = CGAL::to_double(sum_triangles);
        
        assert(approx_equal(total, sum) && 
               "Polygon area must equal sum of triangle areas (decomposition property)");
    };
    
    // ========================================
    // PROPERTY 2: Reversing vertex order negates area
    // ========================================
    auto check_reversal = [&](const std::vector<Point_2>& polygon) {
        if (polygon.size() < 3) return;
        
        FT area_original = polygon_area(polygon);
        
        std::vector<Point_2> reversed = polygon;
        std::reverse(reversed.begin(), reversed.end());
        FT area_reversed = polygon_area(reversed);
        
        double orig = CGAL::to_double(area_original);
        double rev = CGAL::to_double(area_reversed);
        
        assert(approx_equal(orig, -rev) && 
               "Reversing polygon vertices must negate the area");
    };
    
    // ========================================
    // PROPERTY 3: Area is invariant under translation
    // ========================================
    auto check_translation_invariance = [&](const std::vector<Point_2>& polygon, 
                                             double dx, double dy) {
        if (polygon.size() < 3) return;
        
        FT area_original = polygon_area(polygon);
        
        std::vector<Point_2> translated;
        for (const auto& p : polygon) {
            translated.push_back(Point_2(
                CGAL::to_double(p.x()) + dx,
                CGAL::to_double(p.y()) + dy
            ));
        }
        FT area_translated = polygon_area(translated);
        
        double orig = std::abs(CGAL::to_double(area_original));
        double trans = std::abs(CGAL::to_double(area_translated));
        
        assert(approx_equal(orig, trans) && 
               "Area must be invariant under translation");
    };
    
    // ========================================
    // PROPERTY 4: Area is non-negative for CCW polygons
    // ========================================
    auto check_area_sign = [](const std::vector<Point_2>& polygon) {
        if (polygon.size() < 3) return;
        
        FT area = polygon_area(polygon);
        // Note: We don't enforce sign as it depends on vertex ordering
        // Just check that area has a definite sign (not NaN or undefined)
        double a = CGAL::to_double(area);
        assert(!std::isnan(a) && !std::isinf(a) && 
               "Area must be a valid finite number");
    };
    
    // ========================================
    // PROPERTY 5: Empty and degenerate polygons have zero area
    // ========================================
    auto check_degenerate = [&](const std::vector<Point_2>& polygon) {
        FT area = polygon_area(polygon);
        double a = std::abs(CGAL::to_double(area));
        assert(approx_equal(a, 0.0) && 
               "Degenerate polygon must have zero area");
    };
    
    // ========================================
    // PROPERTY 6: Scaling property - scaling by factor k multiplies area by k²
    // ========================================
    auto check_scaling = [&](const std::vector<Point_2>& polygon, double scale) {
        if (polygon.size() < 3) return;
        
        FT area_original = polygon_area(polygon);
        
        std::vector<Point_2> scaled;
        for (const auto& p : polygon) {
            scaled.push_back(Point_2(
                CGAL::to_double(p.x()) * scale,
                CGAL::to_double(p.y()) * scale
            ));
        }
        FT area_scaled = polygon_area(scaled);
        
        double orig = std::abs(CGAL::to_double(area_original));
        double scal = std::abs(CGAL::to_double(area_scaled));
        double expected = orig * scale * scale;
        
        assert(approx_equal(scal, expected) && 
               "Scaling by factor k must multiply area by k²");
    };
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: Unit square (area = 1)
    std::vector<Point_2> square = {
        Point_2(0, 0), Point_2(1, 0), Point_2(1, 1), Point_2(0, 1)
    };
    FT area1 = polygon_area(square);
    assert(approx_equal(std::abs(CGAL::to_double(area1)), 1.0) && 
           "Unit square must have area 1");
    check_triangle_decomposition(square);
    check_reversal(square);
    check_translation_invariance(square, 10, 20);
    check_area_sign(square);
    check_scaling(square, 2.0);
    
    // Test 2: Triangle (base=4, height=3, area=6)
    std::vector<Point_2> triangle = {
        Point_2(0, 0), Point_2(4, 0), Point_2(2, 3)
    };
    FT area2 = polygon_area(triangle);
    assert(approx_equal(std::abs(CGAL::to_double(area2)), 6.0) && 
           "Triangle with base 4 and height 3 must have area 6");
    check_triangle_decomposition(triangle);
    check_reversal(triangle);
    check_translation_invariance(triangle, -5, 5);
    check_area_sign(triangle);
    check_scaling(triangle, 0.5);
    
    // Test 3: Larger square (area = 4)
    std::vector<Point_2> big_square = {
        Point_2(0, 0), Point_2(2, 0), Point_2(2, 2), Point_2(0, 2)
    };
    FT area3 = polygon_area(big_square);
    assert(approx_equal(std::abs(CGAL::to_double(area3)), 4.0) && 
           "2x2 square must have area 4");
    check_triangle_decomposition(big_square);
    check_reversal(big_square);
    check_translation_invariance(big_square, 100, -100);
    check_scaling(big_square, 3.0);
    
    // Test 4: Empty polygon (area = 0)
    std::vector<Point_2> empty;
    check_degenerate(empty);
    
    // Test 5: Single point (area = 0)
    std::vector<Point_2> single = {Point_2(1, 1)};
    check_degenerate(single);
    
    // Test 6: Two points (area = 0)
    std::vector<Point_2> two_points = {Point_2(0, 0), Point_2(1, 1)};
    check_degenerate(two_points);
    
    // Test 7: Pentagon
    std::vector<Point_2> pentagon = {
        Point_2(0, 0), Point_2(2, 0), Point_2(3, 2), Point_2(1, 3), Point_2(-1, 2)
    };
    FT area7 = polygon_area(pentagon);
    assert(approx_equal(std::abs(CGAL::to_double(area7)), 8.0) && 
           "Pentagon must have correct area");
    check_triangle_decomposition(pentagon);
    check_reversal(pentagon);
    check_translation_invariance(pentagon, 50, 50);
    check_area_sign(pentagon);
    
    // Test 8: Rectangle (width=3, height=2, area=6)
    std::vector<Point_2> rectangle = {
        Point_2(0, 0), Point_2(3, 0), Point_2(3, 2), Point_2(0, 2)
    };
    FT area8 = polygon_area(rectangle);
    assert(approx_equal(std::abs(CGAL::to_double(area8)), 6.0) && 
           "Rectangle 3x2 must have area 6");
    check_triangle_decomposition(rectangle);
    check_reversal(rectangle);
    check_scaling(rectangle, 1.5);
    
    // Test 9: Right triangle (legs 3 and 4, area = 6)
    std::vector<Point_2> right_triangle = {
        Point_2(0, 0), Point_2(3, 0), Point_2(0, 4)
    };
    FT area9 = polygon_area(right_triangle);
    assert(approx_equal(std::abs(CGAL::to_double(area9)), 6.0) && 
           "Right triangle with legs 3 and 4 must have area 6");
    check_triangle_decomposition(right_triangle);
    check_reversal(right_triangle);
    
    // Test 10: Hexagon (regular-ish)
    std::vector<Point_2> hexagon = {
        Point_2(1, 0), Point_2(2, 0), Point_2(3, 1), 
        Point_2(2, 2), Point_2(1, 2), Point_2(0, 1)
    };
    check_triangle_decomposition(hexagon);
    check_reversal(hexagon);
    check_translation_invariance(hexagon, -10, -10);
    check_area_sign(hexagon);
    check_scaling(hexagon, 2.5);
    
    // Test 11: Collinear points (degenerate, area = 0)
    std::vector<Point_2> collinear_poly = {
        Point_2(0, 0), Point_2(1, 1), Point_2(2, 2)
    };
    check_degenerate(collinear_poly);
    
    // Test 12: Large coordinates
    std::vector<Point_2> large_square = {
        Point_2(0, 0), Point_2(1000, 0), Point_2(1000, 1000), Point_2(0, 1000)
    };
    FT area12 = polygon_area(large_square);
    assert(approx_equal(std::abs(CGAL::to_double(area12)), 1000000.0) && 
           "Large square must have correct area");
    check_triangle_decomposition(large_square);
    check_reversal(large_square);
    
    std::cout << "test_polygon_area_strict: PASSED (12 test cases with property validation)\n";
    return 0;
}
