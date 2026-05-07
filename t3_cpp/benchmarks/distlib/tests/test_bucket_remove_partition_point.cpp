#include "../src/distlib_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <set>

int main() {
    constexpr float TOLERANCE = 0.001f;
    
    // Test 1: Remove from middle - verify remaining elements
    std::vector<float> points1 = {0.2f, 0.4f, 0.6f, 0.8f};
    std::vector<float> expected1 = {0.2f, 0.4f, 0.8f};
    distlib::bucket_remove_partition_point(points1, 2);
    assert(points1.size() == 3 && "Should have 3 points after removal");
    // Verify exact remaining elements (generalized check)
    for (size_t i = 0; i < points1.size(); ++i) {
        assert(std::abs(points1[i] - expected1[i]) < TOLERANCE && "Remaining elements should match expected");
    }
    // Verify removed element is gone
    bool found_removed = false;
    for (auto p : points1) {
        if (std::abs(p - 0.6f) < TOLERANCE) found_removed = true;
    }
    assert(!found_removed && "0.6 should be removed");
    
    // Test 2: Remove first element - verify shift
    std::vector<float> points2 = {0.2f, 0.4f, 0.6f};
    std::vector<float> expected2 = {0.4f, 0.6f};
    distlib::bucket_remove_partition_point(points2, 0);
    assert(points2.size() == 2 && "Should have 2 points");
    for (size_t i = 0; i < points2.size(); ++i) {
        assert(std::abs(points2[i] - expected2[i]) < TOLERANCE && "Elements should shift correctly");
    }
    
    // Test 3: Remove last element - verify no other changes
    std::vector<float> points3 = {0.2f, 0.4f, 0.6f};
    std::vector<float> expected3 = {0.2f, 0.4f};
    distlib::bucket_remove_partition_point(points3, 2);
    assert(points3.size() == 2 && "Should have 2 points");
    for (size_t i = 0; i < points3.size(); ++i) {
        assert(std::abs(points3[i] - expected3[i]) < TOLERANCE && "Other elements unchanged");
    }
    
    // Test 4: Invalid index (out of bounds) - should do nothing
    std::vector<float> points4 = {0.2f, 0.4f};
    std::vector<float> backup4 = points4;
    distlib::bucket_remove_partition_point(points4, 10);
    assert(points4.size() == backup4.size() && "Size should not change for invalid index");
    assert(points4 == backup4 && "Vector should be unchanged for invalid index");
    
    // Test 5: Remove from single element vector
    std::vector<float> points5 = {0.5f};
    distlib::bucket_remove_partition_point(points5, 0);
    assert(points5.empty() && "Should be empty after removing only element");
    
    // Test 6: Remove all elements sequentially
    std::vector<float> points6 = {0.2f, 0.4f, 0.6f};
    distlib::bucket_remove_partition_point(points6, 0);
    assert(points6.size() == 2 && "Should have 2 after first removal");
    distlib::bucket_remove_partition_point(points6, 0);
    assert(points6.size() == 1 && "Should have 1 after second removal");
    distlib::bucket_remove_partition_point(points6, 0);
    assert(points6.empty() && "Should be empty after removing all");
    
    // Test 7: Remove from two-element vector
    std::vector<float> points7a = {0.3f, 0.7f};
    distlib::bucket_remove_partition_point(points7a, 0);
    assert(points7a.size() == 1 && "Should have 1 element");
    assert(std::abs(points7a[0] - 0.7f) < TOLERANCE && "Should keep second element");
    
    std::vector<float> points7b = {0.3f, 0.7f};
    distlib::bucket_remove_partition_point(points7b, 1);
    assert(points7b.size() == 1 && "Should have 1 element");
    assert(std::abs(points7b[0] - 0.3f) < TOLERANCE && "Should keep first element");
    
    // Test 8: Verify order preservation after removal
    std::vector<float> points8 = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f};
    distlib::bucket_remove_partition_point(points8, 2);  // Remove 0.3
    assert(points8.size() == 4 && "Should have 4 elements");
    assert(std::is_sorted(points8.begin(), points8.end()) && "Should remain sorted");
    // Verify specific elements
    std::vector<float> expected8 = {0.1f, 0.2f, 0.4f, 0.5f};
    for (size_t i = 0; i < points8.size(); ++i) {
        assert(std::abs(points8[i] - expected8[i]) < TOLERANCE && "Order should be preserved");
    }
    
    std::cout << "All bucket_remove_partition_point tests passed! (8 tests)" << std::endl;
    return 0;
}
