#include "../src/octree_functions.h"
#include <cassert>
#include <cmath>
#include <vector>
#include <algorithm>

using namespace OrthoTree;

constexpr double TOLERANCE = 1e-9;

// Property 1: Non-negativity - distance must always be >= 0
bool check_non_negativity(const Point3D& p1, const Point3D& p2) {
    double dist = SquaredDistance(p1, p2);
    return dist >= -TOLERANCE;
}

// Property 2: Identity - distance from point to itself is 0
bool check_identity(const Point3D& p) {
    double dist = SquaredDistance(p, p);
    return std::abs(dist) < TOLERANCE;
}

// Property 3: Symmetry - distance(A,B) == distance(B,A)
bool check_symmetry(const Point3D& p1, const Point3D& p2) {
    double dist1 = SquaredDistance(p1, p2);
    double dist2 = SquaredDistance(p2, p1);
    return std::abs(dist1 - dist2) < TOLERANCE;
}

// Property 4: Triangle Inequality - d(A,C) <= d(A,B) + d(B,C)
// For squared distances: sqrt(d²(A,C)) <= sqrt(d²(A,B)) + sqrt(d²(B,C))
bool check_triangle_inequality(const Point3D& p1, const Point3D& p2, const Point3D& p3) {
    double d12_sq = SquaredDistance(p1, p2);
    double d23_sq = SquaredDistance(p2, p3);
    double d13_sq = SquaredDistance(p1, p3);
    
    double d12 = std::sqrt(d12_sq);
    double d23 = std::sqrt(d23_sq);
    double d13 = std::sqrt(d13_sq);
    
    // Allow small tolerance for floating-point errors
    return d13 <= d12 + d23 + TOLERANCE;
}

// Property 5: Scaling - distance scales quadratically with coordinate scaling
bool check_scaling_property(const Point3D& p1, const Point3D& p2, double scale) {
    double original_dist = SquaredDistance(p1, p2);
    
    Point3D scaled_p1{p1[0] * scale, p1[1] * scale, p1[2] * scale};
    Point3D scaled_p2{p2[0] * scale, p2[1] * scale, p2[2] * scale};
    double scaled_dist = SquaredDistance(scaled_p1, scaled_p2);
    
    // Squared distance should scale by scale²
    double expected = original_dist * scale * scale;
    return std::abs(scaled_dist - expected) < TOLERANCE * std::abs(expected) + TOLERANCE;
}

// Property 6: Translation invariance - distance unchanged by translation
bool check_translation_invariance(const Point3D& p1, const Point3D& p2, const Point3D& offset) {
    double original_dist = SquaredDistance(p1, p2);
    
    Point3D translated_p1{p1[0] + offset[0], p1[1] + offset[1], p1[2] + offset[2]};
    Point3D translated_p2{p2[0] + offset[0], p2[1] + offset[1], p2[2] + offset[2]};
    double translated_dist = SquaredDistance(translated_p1, translated_p2);
    
    return std::abs(original_dist - translated_dist) < TOLERANCE;
}

// Property 7: Pythagorean theorem for axis-aligned distances
bool check_pythagorean_property(const Point3D& p1, const Point3D& p2) {
    double dist_sq = SquaredDistance(p1, p2);
    
    double dx = p2[0] - p1[0];
    double dy = p2[1] - p1[1];
    double dz = p2[2] - p1[2];
    double expected = dx*dx + dy*dy + dz*dz;
    
    return std::abs(dist_sq - expected) < TOLERANCE;
}

// Property 8: Monotonicity - moving point away increases distance
bool check_monotonicity(const Point3D& origin, const Point3D& p, double factor) {
    assert(factor > 1.0);
    
    double dist1 = SquaredDistance(origin, p);
    
    // Move point farther away by scaling from origin
    Point3D farther{p[0] * factor, p[1] * factor, p[2] * factor};
    double dist2 = SquaredDistance(origin, farther);
    
    return dist2 >= dist1 - TOLERANCE;
}

int main() {
    // Test point pairs
    std::vector<std::pair<Point3D, Point3D>> test_pairs = {
        // Basic cases
        {{0.0, 0.0, 0.0}, {1.0, 1.0, 1.0}},
        {{0.0, 0.0, 0.0}, {3.0, 4.0, 0.0}},  // 3-4-5 triangle
        {{0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}},  // Single axis
        
        // Negative coordinates
        {{-1.0, -1.0, -1.0}, {1.0, 1.0, 1.0}},
        {{-5.0, -3.0, -2.0}, {-1.0, -1.0, -1.0}},
        
        // Large distances
        {{0.0, 0.0, 0.0}, {100.0, 100.0, 100.0}},
        {{1000.0, 2000.0, 3000.0}, {1100.0, 2200.0, 3300.0}},
        
        // Small distances
        {{0.0, 0.0, 0.0}, {1e-6, 1e-6, 1e-6}},
        {{1.0, 1.0, 1.0}, {1.000001, 1.000001, 1.000001}},
        
        // Mixed scales
        {{0.001, 1000.0, -500.0}, {0.002, 2000.0, 500.0}},
        
        // Asymmetric
        {{-10.0, 5.0, -3.0}, {20.0, 15.0, 7.0}}
    };
    
    // Test all properties for all point pairs
    for (size_t i = 0; i < test_pairs.size(); ++i) {
        const auto& [p1, p2] = test_pairs[i];
        
        // Property 1: Non-negativity
        assert(check_non_negativity(p1, p2) && 
               "Property 1 failed: Distance must be non-negative");
        
        // Property 2: Identity (test both points)
        assert(check_identity(p1) && 
               "Property 2 failed: Distance from point to itself must be 0");
        assert(check_identity(p2) && 
               "Property 2 failed: Distance from point to itself must be 0");
        
        // Property 3: Symmetry
        assert(check_symmetry(p1, p2) && 
               "Property 3 failed: Distance must be symmetric");
        
        // Property 5: Scaling (test multiple scales)
        assert(check_scaling_property(p1, p2, 2.0) && 
               "Property 5 failed: Scaling property (scale=2.0)");
        assert(check_scaling_property(p1, p2, 0.5) && 
               "Property 5 failed: Scaling property (scale=0.5)");
        assert(check_scaling_property(p1, p2, 10.0) && 
               "Property 5 failed: Scaling property (scale=10.0)");
        
        // Property 6: Translation invariance
        assert(check_translation_invariance(p1, p2, Point3D{10.0, 20.0, 30.0}) && 
               "Property 6 failed: Translation invariance");
        assert(check_translation_invariance(p1, p2, Point3D{-5.0, -10.0, -15.0}) && 
               "Property 6 failed: Translation invariance (negative)");
        
        // Property 7: Pythagorean theorem
        assert(check_pythagorean_property(p1, p2) && 
               "Property 7 failed: Pythagorean theorem");
    }
    
    // Property 4: Triangle Inequality (test with triplets)
    std::vector<std::tuple<Point3D, Point3D, Point3D>> triangle_tests = {
        {{0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}},
        {{0.0, 0.0, 0.0}, {1.0, 1.0, 1.0}, {2.0, 2.0, 2.0}},
        {{-1.0, -1.0, -1.0}, {0.0, 0.0, 0.0}, {1.0, 1.0, 1.0}},
        {{0.0, 0.0, 0.0}, {3.0, 0.0, 0.0}, {0.0, 4.0, 0.0}},
        {{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, {7.0, 8.0, 9.0}},
        {{-10.0, 5.0, -3.0}, {0.0, 0.0, 0.0}, {20.0, 15.0, 7.0}}
    };
    
    for (const auto& [p1, p2, p3] : triangle_tests) {
        assert(check_triangle_inequality(p1, p2, p3) && 
               "Property 4 failed: Triangle inequality");
        // Test all permutations
        assert(check_triangle_inequality(p2, p3, p1) && 
               "Property 4 failed: Triangle inequality (permutation)");
        assert(check_triangle_inequality(p3, p1, p2) && 
               "Property 4 failed: Triangle inequality (permutation)");
    }
    
    // Property 8: Monotonicity
    Point3D origin{0.0, 0.0, 0.0};
    std::vector<Point3D> monotonicity_points = {
        {1.0, 0.0, 0.0},
        {1.0, 1.0, 1.0},
        {3.0, 4.0, 5.0},
        {-2.0, -3.0, -4.0}
    };
    
    for (const auto& p : monotonicity_points) {
        assert(check_monotonicity(origin, p, 2.0) && 
               "Property 8 failed: Monotonicity (factor=2.0)");
        assert(check_monotonicity(origin, p, 1.5) && 
               "Property 8 failed: Monotonicity (factor=1.5)");
    }
    
    // Specific known values (with tolerance)
    double dist1 = SquaredDistance(Point3D{0.0, 0.0, 0.0}, Point3D{1.0, 1.0, 1.0});
    assert(std::abs(dist1 - 3.0) < TOLERANCE && 
           "Known value test failed: (0,0,0) to (1,1,1) should be 3.0");
    
    double dist2 = SquaredDistance(Point3D{0.0, 0.0, 0.0}, Point3D{3.0, 4.0, 0.0});
    assert(std::abs(dist2 - 25.0) < TOLERANCE && 
           "Known value test failed: 3-4-5 triangle should be 25.0");
    
    return 0;
}
