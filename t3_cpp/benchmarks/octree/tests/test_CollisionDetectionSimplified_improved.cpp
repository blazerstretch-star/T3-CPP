#include "../src/octree_functions.h"
#include <cassert>
#include <algorithm>
#include <set>
#include <vector>

using namespace OrthoTree;

constexpr double TOLERANCE = 1e-9;

// Helper: Check if two boxes overlap
bool boxes_overlap(const BoundingBox3D& box1, const BoundingBox3D& box2) {
    for (int i = 0; i < 3; ++i) {
        if (box1.Max[i] < box2.Min[i] - TOLERANCE || box1.Min[i] > box2.Max[i] + TOLERANCE) {
            return false;
        }
    }
    return true;
}

// Property 1: All returned indices must be valid
bool check_valid_indices(const std::vector<std::pair<size_t, size_t>>& result, size_t box_count) {
    for (const auto& [i, j] : result) {
        if (i >= box_count || j >= box_count) {
            return false;
        }
    }
    return true;
}

// Property 2: All returned pairs must actually overlap
bool check_all_overlap(const std::vector<BoundingBox3D>& boxes,
                       const std::vector<std::pair<size_t, size_t>>& result) {
    for (const auto& [i, j] : result) {
        if (!boxes_overlap(boxes[i], boxes[j])) {
            return false;
        }
    }
    return true;
}

// Property 3: No self-collisions (i != j)
bool check_no_self_collision(const std::vector<std::pair<size_t, size_t>>& result) {
    for (const auto& [i, j] : result) {
        if (i == j) {
            return false;
        }
    }
    return true;
}

// Property 4: No duplicate pairs (considering both (i,j) and (j,i) as same)
bool check_no_duplicates(const std::vector<std::pair<size_t, size_t>>& result) {
    std::set<std::pair<size_t, size_t>> normalized_pairs;
    
    for (const auto& [i, j] : result) {
        auto normalized = std::minmax(i, j);
        if (!normalized_pairs.insert(normalized).second) {
            return false; // Duplicate found
        }
    }
    return true;
}

// Property 5: Completeness - all overlapping pairs must be returned
bool check_completeness(const std::vector<BoundingBox3D>& boxes,
                        const std::vector<std::pair<size_t, size_t>>& result) {
    std::set<std::pair<size_t, size_t>> result_set;
    for (const auto& [i, j] : result) {
        result_set.insert(std::minmax(i, j));
    }
    
    // Check all possible pairs
    for (size_t i = 0; i < boxes.size(); ++i) {
        for (size_t j = i + 1; j < boxes.size(); ++j) {
            bool overlaps = boxes_overlap(boxes[i], boxes[j]);
            bool in_result = result_set.count({i, j}) > 0;
            
            if (overlaps != in_result) {
                return false;
            }
        }
    }
    return true;
}

// Property 6: Symmetry - if (i,j) is returned, (j,i) should not be (avoid duplicates)
bool check_symmetry(const std::vector<std::pair<size_t, size_t>>& result) {
    std::set<std::pair<size_t, size_t>> pairs;
    
    for (const auto& [i, j] : result) {
        // Check if reverse pair exists
        if (pairs.count({j, i}) > 0) {
            return false;
        }
        pairs.insert({i, j});
    }
    return true;
}

// Property 7: Order consistency - pairs should maintain some order (i < j or j < i consistently)
bool check_order_consistency(const std::vector<std::pair<size_t, size_t>>& result) {
    // This is a weak property - just check that pairs are ordered in some way
    // Most implementations will use i < j
    for (const auto& [i, j] : result) {
        // Just verify it's not self-collision
        if (i == j) return false;
    }
    return true;
}

// Property 8: Empty input returns empty result
bool check_empty_input() {
    std::vector<BoundingBox3D> empty_boxes;
    auto result = CollisionDetectionSimplified(empty_boxes);
    return result.empty();
}

// Property 9: Single box returns empty result (no collisions with itself)
bool check_single_box(const BoundingBox3D& box) {
    std::vector<BoundingBox3D> single = {box};
    auto result = CollisionDetectionSimplified(single);
    return result.empty();
}

// Property 10: Translation invariance - collisions unchanged by translation
bool check_translation_invariance(const std::vector<BoundingBox3D>& boxes,
                                   const Point3D& offset) {
    auto result1 = CollisionDetectionSimplified(boxes);
    
    // Translate all boxes
    std::vector<BoundingBox3D> translated_boxes;
    for (const auto& box : boxes) {
        translated_boxes.push_back({
            Point3D{box.Min[0] + offset[0], box.Min[1] + offset[1], box.Min[2] + offset[2]},
            Point3D{box.Max[0] + offset[0], box.Max[1] + offset[1], box.Max[2] + offset[2]}
        });
    }
    
    auto result2 = CollisionDetectionSimplified(translated_boxes);
    
    // Results should be identical (same pairs)
    if (result1.size() != result2.size()) return false;
    
    std::set<std::pair<size_t, size_t>> set1, set2;
    for (const auto& p : result1) set1.insert(std::minmax(p.first, p.second));
    for (const auto& p : result2) set2.insert(std::minmax(p.first, p.second));
    
    return set1 == set2;
}

int main() {
    // Test dataset 1: Two overlapping boxes
    std::vector<BoundingBox3D> boxes1 = {
        {Point3D{0.0, 0.0, 0.0}, Point3D{2.0, 2.0, 2.0}},
        {Point3D{1.0, 1.0, 1.0}, Point3D{3.0, 3.0, 3.0}}
    };
    
    // Test dataset 2: No collisions
    std::vector<BoundingBox3D> boxes2 = {
        {Point3D{0.0, 0.0, 0.0}, Point3D{1.0, 1.0, 1.0}},
        {Point3D{2.0, 2.0, 2.0}, Point3D{3.0, 3.0, 3.0}},
        {Point3D{4.0, 4.0, 4.0}, Point3D{5.0, 5.0, 5.0}}
    };
    
    // Test dataset 3: Multiple collisions (chain)
    std::vector<BoundingBox3D> boxes3 = {
        {Point3D{0.0, 0.0, 0.0}, Point3D{3.0, 3.0, 3.0}},
        {Point3D{1.0, 1.0, 1.0}, Point3D{4.0, 4.0, 4.0}},
        {Point3D{2.0, 2.0, 2.0}, Point3D{5.0, 5.0, 5.0}}
    };
    
    // Test dataset 4: One box contains another
    std::vector<BoundingBox3D> boxes4 = {
        {Point3D{0.0, 0.0, 0.0}, Point3D{10.0, 10.0, 10.0}},
        {Point3D{2.0, 2.0, 2.0}, Point3D{5.0, 5.0, 5.0}}
    };
    
    // Test dataset 5: Adjacent boxes (touching but not overlapping)
    std::vector<BoundingBox3D> boxes5 = {
        {Point3D{0.0, 0.0, 0.0}, Point3D{1.0, 1.0, 1.0}},
        {Point3D{1.0, 0.0, 0.0}, Point3D{2.0, 1.0, 1.0}}
    };
    
    // Test dataset 6: All boxes overlap with each other
    std::vector<BoundingBox3D> boxes6 = {
        {Point3D{0.0, 0.0, 0.0}, Point3D{2.0, 2.0, 2.0}},
        {Point3D{0.5, 0.5, 0.5}, Point3D{2.5, 2.5, 2.5}},
        {Point3D{1.0, 1.0, 1.0}, Point3D{3.0, 3.0, 3.0}}
    };
    
    // Test dataset 7: Complex scenario
    std::vector<BoundingBox3D> boxes7 = {
        {Point3D{0.0, 0.0, 0.0}, Point3D{1.0, 1.0, 1.0}},
        {Point3D{0.5, 0.5, 0.5}, Point3D{1.5, 1.5, 1.5}},
        {Point3D{2.0, 2.0, 2.0}, Point3D{3.0, 3.0, 3.0}},
        {Point3D{2.5, 2.5, 2.5}, Point3D{3.5, 3.5, 3.5}},
        {Point3D{10.0, 10.0, 10.0}, Point3D{11.0, 11.0, 11.0}}
    };
    
    // Test dataset 8: Negative coordinates
    std::vector<BoundingBox3D> boxes8 = {
        {Point3D{-5.0, -5.0, -5.0}, Point3D{-2.0, -2.0, -2.0}},
        {Point3D{-4.0, -4.0, -4.0}, Point3D{-1.0, -1.0, -1.0}},
        {Point3D{-10.0, -10.0, -10.0}, Point3D{-8.0, -8.0, -8.0}}
    };
    
    std::vector<std::vector<BoundingBox3D>> all_datasets = {
        boxes1, boxes2, boxes3, boxes4, boxes5, boxes6, boxes7, boxes8
    };
    
    // Test all datasets
    for (size_t ds_idx = 0; ds_idx < all_datasets.size(); ++ds_idx) {
        const auto& boxes = all_datasets[ds_idx];
        auto result = CollisionDetectionSimplified(boxes);
        
        // Property 1: Valid indices
        assert(check_valid_indices(result, boxes.size()) &&
               "Property 1 failed: All indices must be valid");
        
        // Property 2: All returned pairs overlap
        assert(check_all_overlap(boxes, result) &&
               "Property 2 failed: All returned pairs must overlap");
        
        // Property 3: No self-collisions
        assert(check_no_self_collision(result) &&
               "Property 3 failed: No self-collisions allowed");
        
        // Property 4: No duplicates
        assert(check_no_duplicates(result) &&
               "Property 4 failed: No duplicate pairs allowed");
        
        // Property 5: Completeness
        assert(check_completeness(boxes, result) &&
               "Property 5 failed: All overlapping pairs must be returned");
        
        // Property 6: Symmetry
        assert(check_symmetry(result) &&
               "Property 6 failed: No reverse duplicates allowed");
        
        // Property 7: Order consistency
        assert(check_order_consistency(result) &&
               "Property 7 failed: Pairs must be consistently ordered");
        
        // Property 10: Translation invariance
        assert(check_translation_invariance(boxes, Point3D{10.0, 20.0, 30.0}) &&
               "Property 10 failed: Translation invariance");
    }
    
    // Property 8: Empty input
    assert(check_empty_input() &&
           "Property 8 failed: Empty input must return empty result");
    
    // Property 9: Single box
    assert(check_single_box({Point3D{0.0, 0.0, 0.0}, Point3D{1.0, 1.0, 1.0}}) &&
           "Property 9 failed: Single box must return empty result");
    
    // Edge case: Identical boxes
    std::vector<BoundingBox3D> identical_boxes = {
        {Point3D{1.0, 1.0, 1.0}, Point3D{2.0, 2.0, 2.0}},
        {Point3D{1.0, 1.0, 1.0}, Point3D{2.0, 2.0, 2.0}}
    };
    auto result_identical = CollisionDetectionSimplified(identical_boxes);
    assert(result_identical.size() == 1 &&
           "Identical boxes must collide");
    
    // Edge case: Point boxes (zero volume)
    std::vector<BoundingBox3D> point_boxes = {
        {Point3D{1.0, 1.0, 1.0}, Point3D{1.0, 1.0, 1.0}},
        {Point3D{1.0, 1.0, 1.0}, Point3D{1.0, 1.0, 1.0}},
        {Point3D{2.0, 2.0, 2.0}, Point3D{2.0, 2.0, 2.0}}
    };
    auto result_points = CollisionDetectionSimplified(point_boxes);
    // First two point boxes at same location should collide
    assert(check_completeness(point_boxes, result_points) &&
           "Point boxes at same location should collide");
    
    // Verify specific known case
    std::vector<BoundingBox3D> known_boxes = {
        {Point3D{0.0, 0.0, 0.0}, Point3D{2.0, 2.0, 2.0}},
        {Point3D{1.0, 1.0, 1.0}, Point3D{3.0, 3.0, 3.0}},
        {Point3D{5.0, 5.0, 5.0}, Point3D{6.0, 6.0, 6.0}}
    };
    auto known_result = CollisionDetectionSimplified(known_boxes);
    assert(known_result.size() == 1 &&
           "Known case: Should have exactly 1 collision");
    
    // Verify the collision is between boxes 0 and 1
    std::set<std::pair<size_t, size_t>> known_set;
    for (const auto& p : known_result) {
        known_set.insert(std::minmax(p.first, p.second));
    }
    assert(known_set.count({0, 1}) == 1 &&
           "Known case: Collision should be between boxes 0 and 1");
    
    return 0;
}
