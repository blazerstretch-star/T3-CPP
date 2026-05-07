#include "../src/octree_functions.h"
#include <cassert>
#include <algorithm>
#include <set>
#include <vector>
#include <cmath>

using namespace OrthoTree;

constexpr double TOLERANCE = 1e-9;

// Helper: Calculate actual distance
double calculate_distance(const Point3D& p1, const Point3D& p2) {
    return std::sqrt(SquaredDistance(p1, p2));
}

// Property 1: Result size must be min(k, points.size())
bool check_result_size(const std::vector<size_t>& result, size_t k, size_t point_count) {
    size_t expected_size = std::min(k, point_count);
    return result.size() == expected_size;
}

// Property 2: All returned indices must be valid
bool check_valid_indices(const std::vector<size_t>& result, size_t point_count) {
    for (size_t idx : result) {
        if (idx >= point_count) {
            return false;
        }
    }
    return true;
}

// Property 3: No duplicate indices
bool check_no_duplicates(const std::vector<size_t>& result) {
    std::set<size_t> unique(result.begin(), result.end());
    return unique.size() == result.size();
}

// Property 4: Results must be sorted by distance (ascending)
bool check_sorted_by_distance(const std::vector<Point3D>& points,
                               const Point3D& search_point,
                               const std::vector<size_t>& result) {
    for (size_t i = 1; i < result.size(); ++i) {
        double dist_prev = calculate_distance(search_point, points[result[i-1]]);
        double dist_curr = calculate_distance(search_point, points[result[i]]);
        
        if (dist_curr < dist_prev - TOLERANCE) {
            return false;
        }
    }
    return true;
}

// Property 5: All returned points must be closer than any non-returned point
bool check_correctness(const std::vector<Point3D>& points,
                       const Point3D& search_point,
                       const std::vector<size_t>& result) {
    if (result.empty() || result.size() == points.size()) {
        return true; // Trivially correct
    }
    
    std::set<size_t> result_set(result.begin(), result.end());
    double max_returned_dist = calculate_distance(search_point, points[result.back()]);
    
    for (size_t i = 0; i < points.size(); ++i) {
        if (result_set.count(i) == 0) {
            double dist = calculate_distance(search_point, points[i]);
            // Non-returned points must be at least as far as the farthest returned point
            if (dist < max_returned_dist - TOLERANCE) {
                return false;
            }
        }
    }
    return true;
}

// Property 6: Increasing k should only add points, not change existing order
bool check_monotonicity(const std::vector<Point3D>& points,
                        const Point3D& search_point,
                        size_t k1, size_t k2) {
    if (k1 >= k2) return true;
    
    auto result1 = GetNearestNeighborsSimplified(points, search_point, k1);
    auto result2 = GetNearestNeighborsSimplified(points, search_point, k2);
    
    // First k1 elements of result2 should match result1
    if (result1.size() > result2.size()) return false;
    
    for (size_t i = 0; i < result1.size(); ++i) {
        if (result1[i] != result2[i]) {
            return false;
        }
    }
    return true;
}

// Property 7: Search point at exact location should return that point first
bool check_exact_match(const std::vector<Point3D>& points,
                       size_t exact_idx) {
    const Point3D& search_point = points[exact_idx];
    auto result = GetNearestNeighborsSimplified(points, search_point, 1);
    
    if (result.empty()) return false;
    
    // The returned point should be at distance 0 (or very close)
    double dist = calculate_distance(search_point, points[result[0]]);
    return dist < TOLERANCE;
}

// Property 8: Translation invariance - relative order unchanged by translation
bool check_translation_invariance(const std::vector<Point3D>& points,
                                   const Point3D& search_point,
                                   const Point3D& offset,
                                   size_t k) {
    auto result1 = GetNearestNeighborsSimplified(points, search_point, k);
    
    // Translate all points and search point
    std::vector<Point3D> translated_points;
    for (const auto& p : points) {
        translated_points.push_back(Point3D{
            p[0] + offset[0],
            p[1] + offset[1],
            p[2] + offset[2]
        });
    }
    Point3D translated_search{
        search_point[0] + offset[0],
        search_point[1] + offset[1],
        search_point[2] + offset[2]
    };
    
    auto result2 = GetNearestNeighborsSimplified(translated_points, translated_search, k);
    
    // Results should be identical (same indices)
    return result1 == result2;
}

int main() {
    // Test dataset 1: Regular grid
    std::vector<Point3D> points1 = {
        Point3D{0.0, 0.0, 0.0},
        Point3D{1.0, 1.0, 1.0},
        Point3D{2.0, 2.0, 2.0},
        Point3D{3.0, 3.0, 3.0},
        Point3D{10.0, 10.0, 10.0}
    };
    
    // Test dataset 2: Random scattered
    std::vector<Point3D> points2 = {
        Point3D{-5.0, 3.0, 7.0},
        Point3D{2.5, -1.5, 4.0},
        Point3D{0.0, 0.0, 0.0},
        Point3D{8.0, -3.0, 2.0},
        Point3D{-2.0, 6.0, -4.0},
        Point3D{4.5, 1.5, -1.0},
        Point3D{-7.0, -2.0, 5.0}
    };
    
    // Test dataset 3: Points with duplicate distances
    std::vector<Point3D> points3 = {
        Point3D{1.0, 0.0, 0.0},
        Point3D{-1.0, 0.0, 0.0},
        Point3D{0.0, 1.0, 0.0},
        Point3D{0.0, -1.0, 0.0},
        Point3D{0.0, 0.0, 1.0},
        Point3D{0.0, 0.0, -1.0}
    };
    
    // Test dataset 4: Clustered points
    std::vector<Point3D> points4 = {
        Point3D{0.0, 0.0, 0.0},
        Point3D{0.1, 0.1, 0.1},
        Point3D{0.2, 0.2, 0.2},
        Point3D{10.0, 10.0, 10.0},
        Point3D{10.1, 10.1, 10.1}
    };
    
    std::vector<std::vector<Point3D>> all_datasets = {points1, points2, points3, points4};
    
    // Test search points
    std::vector<Point3D> search_points = {
        Point3D{0.0, 0.0, 0.0},
        Point3D{1.1, 1.1, 1.1},
        Point3D{5.0, 5.0, 5.0},
        Point3D{-10.0, -10.0, -10.0},
        Point3D{100.0, 100.0, 100.0}
    };
    
    // Test k values
    std::vector<size_t> k_values = {0, 1, 3, 5, 10, 100};
    
    // Test all combinations
    for (const auto& points : all_datasets) {
        for (const auto& search_point : search_points) {
            for (size_t k : k_values) {
                auto result = GetNearestNeighborsSimplified(points, search_point, k);
                
                // Property 1: Result size
                assert(check_result_size(result, k, points.size()) &&
                       "Property 1 failed: Result size must be min(k, points.size())");
                
                // Property 2: Valid indices
                assert(check_valid_indices(result, points.size()) &&
                       "Property 2 failed: All indices must be valid");
                
                // Property 3: No duplicates
                assert(check_no_duplicates(result) &&
                       "Property 3 failed: No duplicate indices allowed");
                
                // Property 4: Sorted by distance
                assert(check_sorted_by_distance(points, search_point, result) &&
                       "Property 4 failed: Results must be sorted by distance");
                
                // Property 5: Correctness (k nearest are actually nearest)
                assert(check_correctness(points, search_point, result) &&
                       "Property 5 failed: Returned points must be k nearest");
            }
        }
    }
    
    // Property 6: Monotonicity
    for (const auto& points : all_datasets) {
        Point3D search{0.0, 0.0, 0.0};
        assert(check_monotonicity(points, search, 1, 3) &&
               "Property 6 failed: Monotonicity (k=1 to k=3)");
        assert(check_monotonicity(points, search, 3, 5) &&
               "Property 6 failed: Monotonicity (k=3 to k=5)");
        assert(check_monotonicity(points, search, 1, points.size()) &&
               "Property 6 failed: Monotonicity (k=1 to all)");
    }
    
    // Property 7: Exact match
    for (const auto& points : all_datasets) {
        for (size_t i = 0; i < points.size(); ++i) {
            assert(check_exact_match(points, i) &&
                   "Property 7 failed: Exact match should return distance 0");
        }
    }
    
    // Property 8: Translation invariance
    for (const auto& points : all_datasets) {
        Point3D search{1.0, 2.0, 3.0};
        Point3D offset{10.0, 20.0, 30.0};
        assert(check_translation_invariance(points, search, offset, 3) &&
               "Property 8 failed: Translation invariance");
    }
    
    // Edge case: Empty point set
    std::vector<Point3D> empty_points;
    auto result_empty = GetNearestNeighborsSimplified(empty_points, Point3D{0.0, 0.0, 0.0}, 5);
    assert(result_empty.empty() &&
           "Empty point set must return empty result");
    
    // Edge case: k = 0
    auto result_k0 = GetNearestNeighborsSimplified(points1, Point3D{0.0, 0.0, 0.0}, 0);
    assert(result_k0.empty() &&
           "k=0 must return empty result");
    
    // Edge case: Single point
    std::vector<Point3D> single_point = {Point3D{5.0, 5.0, 5.0}};
    auto result_single = GetNearestNeighborsSimplified(single_point, Point3D{0.0, 0.0, 0.0}, 1);
    assert(result_single.size() == 1 && result_single[0] == 0 &&
           "Single point must be returned");
    
    // Edge case: k larger than point count
    auto result_large_k = GetNearestNeighborsSimplified(points1, Point3D{0.0, 0.0, 0.0}, 1000);
    assert(result_large_k.size() == points1.size() &&
           "k > point count should return all points");
    
    // Specific known case: Verify actual nearest neighbor
    std::vector<Point3D> known_points = {
        Point3D{0.0, 0.0, 0.0},
        Point3D{1.0, 0.0, 0.0},
        Point3D{2.0, 0.0, 0.0},
        Point3D{10.0, 0.0, 0.0}
    };
    Point3D known_search{0.5, 0.0, 0.0};
    auto known_result = GetNearestNeighborsSimplified(known_points, known_search, 1);
    assert(known_result.size() == 1 &&
           "Known case: Should return 1 neighbor");
    // Nearest should be either index 0 or 1 (both at distance 0.5)
    assert((known_result[0] == 0 || known_result[0] == 1) &&
           "Known case: Nearest should be index 0 or 1");
    
    return 0;
}
