#include "../src/octree_functions.h"
#include <cassert>
#include <cmath>
#include <vector>

using namespace OrthoTree;

// Tolerance for floating-point comparisons
constexpr double TOLERANCE = 1e-9;

// Property 1: Center must be equidistant from min and max in each dimension
bool check_center_equidistant(const BoundingBox3D& box, const Point3D& center) {
    for (int i = 0; i < 3; ++i) {
        double dist_to_min = center[i] - box.Min[i];
        double dist_to_max = box.Max[i] - center[i];
        if (std::abs(dist_to_min - dist_to_max) > TOLERANCE) {
            return false;
        }
    }
    return true;
}

// Property 2: Center must be inside the box (inclusive boundaries)
bool check_center_inside_box(const BoundingBox3D& box, const Point3D& center) {
    for (int i = 0; i < 3; ++i) {
        if (center[i] < box.Min[i] - TOLERANCE || center[i] > box.Max[i] + TOLERANCE) {
            return false;
        }
    }
    return true;
}

// Property 3: Center coordinates must be average of min and max
bool check_center_is_average(const BoundingBox3D& box, const Point3D& center) {
    for (int i = 0; i < 3; ++i) {
        double expected = (box.Min[i] + box.Max[i]) * 0.5;
        if (std::abs(center[i] - expected) > TOLERANCE) {
            return false;
        }
    }
    return true;
}

// Property 4: Scaling invariance - center should scale proportionally
bool check_scaling_invariance(const BoundingBox3D& box, double scale) {
    Point3D center1 = GetBoxCenter(box);
    
    BoundingBox3D scaled_box{
        Point3D{box.Min[0] * scale, box.Min[1] * scale, box.Min[2] * scale},
        Point3D{box.Max[0] * scale, box.Max[1] * scale, box.Max[2] * scale}
    };
    Point3D center2 = GetBoxCenter(scaled_box);
    
    for (int i = 0; i < 3; ++i) {
        if (std::abs(center2[i] - center1[i] * scale) > TOLERANCE * std::abs(scale)) {
            return false;
        }
    }
    return true;
}

// Property 5: Translation invariance - center should translate with box
bool check_translation_invariance(const BoundingBox3D& box, const Point3D& offset) {
    Point3D center1 = GetBoxCenter(box);
    
    BoundingBox3D translated_box{
        Point3D{box.Min[0] + offset[0], box.Min[1] + offset[1], box.Min[2] + offset[2]},
        Point3D{box.Max[0] + offset[0], box.Max[1] + offset[1], box.Max[2] + offset[2]}
    };
    Point3D center2 = GetBoxCenter(translated_box);
    
    for (int i = 0; i < 3; ++i) {
        if (std::abs((center2[i] - center1[i]) - offset[i]) > TOLERANCE) {
            return false;
        }
    }
    return true;
}

int main() {
    std::vector<BoundingBox3D> test_boxes = {
        // Test 1: Standard box
        {Point3D{0.0, 0.0, 0.0}, Point3D{2.0, 4.0, 6.0}},
        
        // Test 2: Negative coordinates
        {Point3D{-4.0, -6.0, -8.0}, Point3D{0.0, 0.0, 0.0}},
        
        // Test 3: Mixed positive/negative (symmetric around origin)
        {Point3D{-1.0, -2.0, -3.0}, Point3D{1.0, 2.0, 3.0}},
        
        // Test 4: Point box (zero volume)
        {Point3D{5.0, 5.0, 5.0}, Point3D{5.0, 5.0, 5.0}},
        
        // Test 5: Large coordinates
        {Point3D{1000.0, 2000.0, 3000.0}, Point3D{1100.0, 2200.0, 3300.0}},
        
        // Test 6: Very small box
        {Point3D{0.0, 0.0, 0.0}, Point3D{1e-6, 1e-6, 1e-6}},
        
        // Test 7: Non-uniform dimensions
        {Point3D{0.0, 0.0, 0.0}, Point3D{1.0, 10.0, 100.0}},
        
        // Test 8: Asymmetric box
        {Point3D{-10.0, 5.0, -3.0}, Point3D{20.0, 15.0, 7.0}},
        
        // Test 9: Large negative coordinates
        {Point3D{-1000.0, -2000.0, -3000.0}, Point3D{-900.0, -1800.0, -2700.0}},
        
        // Test 10: Mixed scales
        {Point3D{0.001, 1000.0, -500.0}, Point3D{0.002, 2000.0, 500.0}}
    };
    
    // Apply all property checks to all test boxes
    for (size_t i = 0; i < test_boxes.size(); ++i) {
        const auto& box = test_boxes[i];
        Point3D center = GetBoxCenter(box);
        
        // Property 1: Equidistant from boundaries
        assert(check_center_equidistant(box, center) && 
               "Property 1 failed: Center must be equidistant from min and max");
        
        // Property 2: Inside box
        assert(check_center_inside_box(box, center) && 
               "Property 2 failed: Center must be inside the box");
        
        // Property 3: Average of min and max
        assert(check_center_is_average(box, center) && 
               "Property 3 failed: Center must be average of min and max");
        
        // Property 4: Scaling invariance (test with multiple scales)
        assert(check_scaling_invariance(box, 2.0) && 
               "Property 4 failed: Scaling invariance (scale=2.0)");
        assert(check_scaling_invariance(box, 0.5) && 
               "Property 4 failed: Scaling invariance (scale=0.5)");
        assert(check_scaling_invariance(box, 10.0) && 
               "Property 4 failed: Scaling invariance (scale=10.0)");
        
        // Property 5: Translation invariance (test with multiple offsets)
        assert(check_translation_invariance(box, Point3D{10.0, 20.0, 30.0}) && 
               "Property 5 failed: Translation invariance (offset=10,20,30)");
        assert(check_translation_invariance(box, Point3D{-5.0, -10.0, -15.0}) && 
               "Property 5 failed: Translation invariance (offset=-5,-10,-15)");
        assert(check_translation_invariance(box, Point3D{0.001, 0.002, 0.003}) && 
               "Property 5 failed: Translation invariance (small offset)");
    }
    
    // Additional edge case: Verify center of symmetric box is at origin
    BoundingBox3D symmetric_box{Point3D{-5.0, -5.0, -5.0}, Point3D{5.0, 5.0, 5.0}};
    Point3D symmetric_center = GetBoxCenter(symmetric_box);
    assert(std::abs(symmetric_center[0]) < TOLERANCE && 
           std::abs(symmetric_center[1]) < TOLERANCE && 
           std::abs(symmetric_center[2]) < TOLERANCE &&
           "Center of symmetric box must be at origin");
    
    return 0;
}
