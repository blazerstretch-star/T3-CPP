#include "../src/octree_functions.h"
#include <cassert>
#include <algorithm>
#include <set>
#include <vector>

using namespace OrthoTree;

constexpr double TOLERANCE = 1e-9;

// Helper: Check if point is inside box (with tolerance)
bool is_point_in_range(const Point3D& point, const BoundingBox3D& range) {
    for (int i = 0; i < 3; ++i) {
        if (point[i] < range.Min[i] - TOLERANCE || point[i] > range.Max[i] + TOLERANCE) {
            return false;
        }
    }
    return true;
}

// Property 1: All returned indices must be valid
bool check_valid_indices(const std::vector<size_t>& result, size_t point_count) {
    for (size_t idx : result) {
        if (idx >= point_count) {
            return false;
        }
    }
    return true;
}

// Property 2: All returned points must be inside the range
bool check_all_inside_range(const std::vector<Point3D>& points, 
                             const std::vector<size_t>& result,
                             const BoundingBox3D& range) {
    for (size_t idx : result) {
        if (!is_point_in_range(points[idx], range)) {
            return false;
        }
    }
    return true;
}

// Property 3: All points inside range must be returned (completeness)
bool check_completeness(const std::vector<Point3D>& points,
                        const std::vector<size_t>& result,
                        const BoundingBox3D& range) {
    std::set<size_t> result_set(result.begin(), result.end());
    
    for (size_t i = 0; i < points.size(); ++i) {
        bool is_inside = is_point_in_range(points[i], range);
        bool is_returned = result_set.count(i) > 0;
        
        if (is_inside != is_returned) {
            return false;
        }
    }
    return true;
}

// Property 4: No duplicate indices in result
bool check_no_duplicates(const std::vector<size_t>& result) {
    std::set<size_t> unique(result.begin(), result.end());
    return unique.size() == result.size();
}

// Property 5: Empty range returns empty result
bool check_empty_range_property(const std::vector<Point3D>& points,
                                 const BoundingBox3D& range) {
    // Check if range has zero volume
    bool is_empty = false;
    for (int i = 0; i < 3; ++i) {
        if (range.Max[i] < range.Min[i] - TOLERANCE) {
            is_empty = true;
            break;
        }
    }
    
    if (is_empty) {
        auto result = RangeSearchSimplified(points, range);
        return result.empty();
    }
    return true;
}

// Property 6: Subset property - smaller range returns subset of results
bool check_subset_property(const std::vector<Point3D>& points,
                           const BoundingBox3D& larger_range,
                           const BoundingBox3D& smaller_range) {
    // Verify smaller_range is actually inside larger_range
    bool is_subset = true;
    for (int i = 0; i < 3; ++i) {
        if (smaller_range.Min[i] < larger_range.Min[i] - TOLERANCE ||
            smaller_range.Max[i] > larger_range.Max[i] + TOLERANCE) {
            is_subset = false;
            break;
        }
    }
    
    if (!is_subset) return true; // Skip if not actually a subset
    
    auto result_larger = RangeSearchSimplified(points, larger_range);
    auto result_smaller = RangeSearchSimplified(points, smaller_range);
    
    std::set<size_t> larger_set(result_larger.begin(), result_larger.end());
    
    // All points in smaller result must be in larger result
    for (size_t idx : result_smaller) {
        if (larger_set.count(idx) == 0) {
            return false;
        }
    }
    
    return true;
}

int main() {
    // Test dataset 1: Regular grid
    std::vector<Point3D> points1 = {
        Point3D{0.0, 0.0, 0.0},
        Point3D{1.0, 1.0, 1.0},
        Point3D{2.0, 2.0, 2.0},
        Point3D{3.0, 3.0, 3.0},
        Point3D{5.0, 5.0, 5.0},
        Point3D{10.0, 10.0, 10.0}
    };
    
    // Test dataset 2: Random scattered points
    std::vector<Point3D> points2 = {
        Point3D{-5.0, 3.0, 7.0},
        Point3D{2.5, -1.5, 4.0},
        Point3D{0.0, 0.0, 0.0},
        Point3D{8.0, -3.0, 2.0},
        Point3D{-2.0, 6.0, -4.0},
        Point3D{4.5, 1.5, -1.0}
    };
    
    // Test dataset 3: Points at boundaries
    std::vector<Point3D> points3 = {
        Point3D{0.0, 0.0, 0.0},
        Point3D{1.0, 0.0, 0.0},
        Point3D{0.0, 1.0, 0.0},
        Point3D{0.0, 0.0, 1.0},
        Point3D{1.0, 1.0, 0.0},
        Point3D{1.0, 0.0, 1.0},
        Point3D{0.0, 1.0, 1.0},
        Point3D{1.0, 1.0, 1.0}
    };
    
    // Test dataset 4: Duplicate points
    std::vector<Point3D> points4 = {
        Point3D{1.0, 1.0, 1.0},
        Point3D{1.0, 1.0, 1.0},
        Point3D{2.0, 2.0, 2.0},
        Point3D{2.0, 2.0, 2.0},
        Point3D{3.0, 3.0, 3.0}
    };
    
    std::vector<std::vector<Point3D>> all_datasets = {points1, points2, points3, points4};
    
    // Test ranges
    std::vector<BoundingBox3D> test_ranges = {
        // Range containing some points
        {Point3D{0.0, 0.0, 0.0}, Point3D{2.5, 2.5, 2.5}},
        
        // Range containing all points (large)
        {Point3D{-100.0, -100.0, -100.0}, Point3D{100.0, 100.0, 100.0}},
        
        // Range containing no points
        {Point3D{20.0, 20.0, 20.0}, Point3D{30.0, 30.0, 30.0}},
        
        // Small range around single point
        {Point3D{0.9, 0.9, 0.9}, Point3D{1.1, 1.1, 1.1}},
        
        // Range at boundary
        {Point3D{0.0, 0.0, 0.0}, Point3D{1.0, 1.0, 1.0}},
        
        // Negative range
        {Point3D{-10.0, -10.0, -10.0}, Point3D{-5.0, -5.0, -5.0}},
        
        // Asymmetric range
        {Point3D{-2.0, 0.0, -5.0}, Point3D{5.0, 10.0, 3.0}},
        
        // Very small range
        {Point3D{0.0, 0.0, 0.0}, Point3D{0.001, 0.001, 0.001}},
        
        // Point range (zero volume)
        {Point3D{1.0, 1.0, 1.0}, Point3D{1.0, 1.0, 1.0}}
    };
    
    // Test all combinations of datasets and ranges
    for (size_t ds_idx = 0; ds_idx < all_datasets.size(); ++ds_idx) {
        const auto& points = all_datasets[ds_idx];
        
        for (size_t r_idx = 0; r_idx < test_ranges.size(); ++r_idx) {
            const auto& range = test_ranges[r_idx];
            
            auto result = RangeSearchSimplified(points, range);
            
            // Property 1: Valid indices
            assert(check_valid_indices(result, points.size()) &&
                   "Property 1 failed: All indices must be valid");
            
            // Property 2: All returned points inside range
            assert(check_all_inside_range(points, result, range) &&
                   "Property 2 failed: All returned points must be inside range");
            
            // Property 3: Completeness
            assert(check_completeness(points, result, range) &&
                   "Property 3 failed: All points inside range must be returned");
            
            // Property 4: No duplicates
            assert(check_no_duplicates(result) &&
                   "Property 4 failed: Result must not contain duplicates");
            
            // Property 5: Empty range
            assert(check_empty_range_property(points, range) &&
                   "Property 5 failed: Empty range should return empty result");
        }
    }
    
    // Property 6: Subset property
    std::vector<std::pair<BoundingBox3D, BoundingBox3D>> subset_tests = {
        {{Point3D{-10.0, -10.0, -10.0}, Point3D{10.0, 10.0, 10.0}},
         {Point3D{-5.0, -5.0, -5.0}, Point3D{5.0, 5.0, 5.0}}},
        
        {{Point3D{0.0, 0.0, 0.0}, Point3D{10.0, 10.0, 10.0}},
         {Point3D{2.0, 2.0, 2.0}, Point3D{8.0, 8.0, 8.0}}},
        
        {{Point3D{-5.0, -5.0, -5.0}, Point3D{5.0, 5.0, 5.0}},
         {Point3D{0.0, 0.0, 0.0}, Point3D{1.0, 1.0, 1.0}}}
    };
    
    for (const auto& [larger, smaller] : subset_tests) {
        for (const auto& points : all_datasets) {
            assert(check_subset_property(points, larger, smaller) &&
                   "Property 6 failed: Subset property");
        }
    }
    
    // Edge case: Empty point set
    std::vector<Point3D> empty_points;
    for (const auto& range : test_ranges) {
        auto result = RangeSearchSimplified(empty_points, range);
        assert(result.empty() && 
               "Empty point set must return empty result");
    }
    
    // Edge case: Single point
    std::vector<Point3D> single_point = {Point3D{5.0, 5.0, 5.0}};
    BoundingBox3D range_contains{Point3D{0.0, 0.0, 0.0}, Point3D{10.0, 10.0, 10.0}};
    BoundingBox3D range_not_contains{Point3D{20.0, 20.0, 20.0}, Point3D{30.0, 30.0, 30.0}};
    
    auto result_contains = RangeSearchSimplified(single_point, range_contains);
    assert(result_contains.size() == 1 && result_contains[0] == 0 &&
           "Single point inside range must be returned");
    
    auto result_not_contains = RangeSearchSimplified(single_point, range_not_contains);
    assert(result_not_contains.empty() &&
           "Single point outside range must not be returned");
    
    return 0;
}
