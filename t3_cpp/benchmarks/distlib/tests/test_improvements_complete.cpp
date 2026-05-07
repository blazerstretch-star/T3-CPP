#include "../src/distlib_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <numeric>

// ============================================================================
// IMPROVED TEST: bucket_remove_partition_point
// Original: 4 tests | Improved: 7 tests | Gap filled: 3 tests
// ============================================================================

int test_bucket_remove_partition_point_improved() {
    // Test 1: Remove from middle
    std::vector<float> points1 = {0.2f, 0.4f, 0.6f, 0.8f};
    distlib::bucket_remove_partition_point(points1, 2);
    assert(points1.size() == 3 && "Should have 3 points");
    for (auto p : points1) {
        assert(std::abs(p - 0.6f) > 0.001f && "0.6 should be removed");
    }
    
    // Test 2: Remove first element
    std::vector<float> points2 = {0.2f, 0.4f, 0.6f};
    distlib::bucket_remove_partition_point(points2, 0);
    assert(points2.size() == 2 && "Should have 2 points");
    assert(std::abs(points2[0] - 0.4f) < 0.001f && "First should now be 0.4");
    
    // Test 3: Remove last element
    std::vector<float> points3 = {0.2f, 0.4f, 0.6f};
    distlib::bucket_remove_partition_point(points3, 2);
    assert(points3.size() == 2 && "Should have 2 points");
    
    // Test 4: Invalid index (should do nothing)
    std::vector<float> points4 = {0.2f, 0.4f};
    distlib::bucket_remove_partition_point(points4, 10);
    assert(points4.size() == 2 && "Should still have 2 points");
    
    // ========================================================================
    // NEW TEST 5: Verify remaining elements after removal (CRITICAL)
    // ========================================================================
    std::vector<float> points5 = {0.1f, 0.2f, 0.3f, 0.4f};
    distlib::bucket_remove_partition_point(points5, 1);  // Remove 0.2
    assert(points5.size() == 3 && "Should have 3 elements");
    assert(std::abs(points5[0] - 0.1f) < 0.001f && "First should be 0.1");
    assert(std::abs(points5[1] - 0.3f) < 0.001f && "Second should be 0.3 (0.2 removed)");
    assert(std::abs(points5[2] - 0.4f) < 0.001f && "Third should be 0.4");
    
    // ========================================================================
    // NEW TEST 6: Remove from single element vector
    // ========================================================================
    std::vector<float> points6 = {0.5f};
    distlib::bucket_remove_partition_point(points6, 0);
    assert(points6.empty() && "Should be empty after removing only element");
    
    // ========================================================================
    // NEW TEST 7: Remove all elements sequentially
    // ========================================================================
    std::vector<float> points7 = {0.2f, 0.4f, 0.6f};
    distlib::bucket_remove_partition_point(points7, 0);
    assert(points7.size() == 2 && "Should have 2 after first removal");
    distlib::bucket_remove_partition_point(points7, 0);
    assert(points7.size() == 1 && "Should have 1 after second removal");
    distlib::bucket_remove_partition_point(points7, 0);
    assert(points7.empty() && "Should be empty after removing all");
    
    std::cout << "All bucket_remove_partition_point tests passed! (7 tests)" << std::endl;
    return 0;
}

// ============================================================================
// IMPROVED TEST: bucket_less_than
// Original: 3 tests | Improved: 8 tests | Gap filled: 5 tests
// ============================================================================

int test_bucket_less_than_improved() {
    std::vector<float> points1 = {0.3f, 0.6f, 0.9f};
    std::vector<float> points2 = {0.3f, 0.6f, 0.8f};
    
    // Test 1: Different names
    bool lt1 = distlib::bucket_less_than("aaa", points1, "bbb", points1);
    assert(lt1 && "aaa should be less than bbb");
    
    bool lt2 = distlib::bucket_less_than("bbb", points1, "aaa", points1);
    assert(!lt2 && "bbb should not be less than aaa");
    
    // Test 2: Same name, different points
    bool lt3 = distlib::bucket_less_than("bucket", points2, "bucket", points1);
    assert(lt3 && "points2 should be less than points1");
    
    // Test 3: Transitivity
    std::vector<float> pa = {0.1f};
    std::vector<float> pb = {0.2f};
    std::vector<float> pc = {0.3f};
    bool a_lt_b = distlib::bucket_less_than("x", pa, "x", pb);
    bool b_lt_c = distlib::bucket_less_than("x", pb, "x", pc);
    bool a_lt_c = distlib::bucket_less_than("x", pa, "x", pc);
    assert(a_lt_b && b_lt_c && a_lt_c && "Less than should be transitive");
    
    // ========================================================================
    // NEW TEST 4: Reflexivity (a < a should be false) - CRITICAL
    // ========================================================================
    std::vector<float> p_reflex = {0.5f, 0.7f};
    bool reflexive = distlib::bucket_less_than("test", p_reflex, "test", p_reflex);
    assert(!reflexive && "Reflexivity: a < a should always be false");
    
    // ========================================================================
    // NEW TEST 5: Epsilon tolerance - within epsilon (CRITICAL)
    // ========================================================================
    std::vector<float> p1 = {0.1f};
    std::vector<float> p2 = {0.1f + 1e-7f};  // Within kEpsilon (1e-6)
    bool within_epsilon = distlib::bucket_less_than("a", p1, "a", p2);
    assert(!within_epsilon && "Within epsilon should be treated as equal");
    
    // ========================================================================
    // NEW TEST 6: Epsilon tolerance - outside epsilon
    // ========================================================================
    std::vector<float> p3 = {0.1f};
    std::vector<float> p4 = {0.1f + 1e-5f};  // Outside kEpsilon
    bool outside_epsilon = distlib::bucket_less_than("a", p3, "a", p4);
    assert(outside_epsilon && "Outside epsilon should compare as less than");
    
    // ========================================================================
    // NEW TEST 7: Empty vectors comparison
    // ========================================================================
    std::vector<float> empty1, empty2;
    bool empty_cmp = distlib::bucket_less_than("a", empty1, "a", empty2);
    assert(!empty_cmp && "Empty vectors should not be less than each other");
    
    // ========================================================================
    // NEW TEST 8: Different length vectors
    // ========================================================================
    std::vector<float> short_vec = {0.5f};
    std::vector<float> long_vec = {0.5f, 0.7f};
    bool short_long = distlib::bucket_less_than("a", short_vec, "a", long_vec);
    // Should compare up to min length, then return false (equal prefix)
    assert(!short_long && "Equal prefix should not be less than");
    
    std::cout << "All bucket_less_than tests passed! (8 tests)" << std::endl;
    return 0;
}

// ============================================================================
// IMPROVED TEST: bucket_add_partition_point
// Original: 8 tests | Improved: 10 tests | Gap filled: 2 tests
// ============================================================================

int test_bucket_add_partition_point_improved() {
    // Test 1: Add to middle
    std::vector<float> points1 = {0.2f, 0.4f, 0.6f, 0.8f};
    distlib::bucket_add_partition_point(points1, 0.5f);
    assert(points1.size() == 5 && "Should have 5 points");
    assert(std::abs(points1[2] - 0.5f) < 0.001f && "0.5 should be at index 2");
    assert(std::is_sorted(points1.begin(), points1.end()) && "Should remain sorted");
    
    // Test 2: Add to beginning
    std::vector<float> points2 = {0.2f, 0.4f, 0.6f};
    distlib::bucket_add_partition_point(points2, 0.1f);
    assert(points2.size() == 4 && "Should have 4 points");
    assert(std::abs(points2[0] - 0.1f) < 0.001f && "0.1 should be at index 0");
    assert(std::is_sorted(points2.begin(), points2.end()));
    
    // Test 3: Add to end
    std::vector<float> points3 = {0.2f, 0.4f, 0.6f};
    distlib::bucket_add_partition_point(points3, 0.9f);
    assert(points3.size() == 4 && "Should have 4 points");
    assert(std::abs(points3[3] - 0.9f) < 0.001f && "0.9 should be at end");
    assert(std::is_sorted(points3.begin(), points3.end()));
    
    // Test 4: Add to empty vector
    std::vector<float> points4;
    distlib::bucket_add_partition_point(points4, 0.5f);
    assert(points4.size() == 1 && "Should have 1 point");
    assert(std::abs(points4[0] - 0.5f) < 0.001f);
    
    // Test 5: Add duplicate (should allow)
    std::vector<float> points5 = {0.2f, 0.4f, 0.6f};
    distlib::bucket_add_partition_point(points5, 0.4f);
    assert(points5.size() == 4 && "Should allow duplicate");
    assert(std::is_sorted(points5.begin(), points5.end()));
    
    // Test 6: Add to single element
    std::vector<float> points6 = {0.5f};
    distlib::bucket_add_partition_point(points6, 0.3f);
    assert(points6.size() == 2);
    assert(points6[0] == 0.3f && points6[1] == 0.5f);
    
    // Test 7: Add multiple in sequence
    std::vector<float> points7;
    distlib::bucket_add_partition_point(points7, 0.5f);
    distlib::bucket_add_partition_point(points7, 0.3f);
    distlib::bucket_add_partition_point(points7, 0.7f);
    distlib::bucket_add_partition_point(points7, 0.1f);
    assert(points7.size() == 4);
    assert(std::is_sorted(points7.begin(), points7.end()));
    assert(points7[0] == 0.1f && points7[3] == 0.7f);
    
    // Test 8: Verify insertion maintains order (not just sorting)
    std::vector<float> points8 = {0.1f, 0.3f, 0.5f, 0.7f, 0.9f};
    distlib::bucket_add_partition_point(points8, 0.6f);
    assert(points8.size() == 6 && "Should have 6 elements");
    assert(std::is_sorted(points8.begin(), points8.end()) && "Should remain sorted");
    auto it = std::find_if(points8.begin(), points8.end(), [](float x) { return std::abs(x - 0.6f) < 0.001f; });
    assert(it != points8.end() && "0.6 should be in vector");
    size_t idx = std::distance(points8.begin(), it);
    assert(idx > 0 && idx < points8.size() - 1 && "0.6 should be in middle");
    assert(points8[idx-1] < 0.6f && points8[idx+1] > 0.6f && "0.6 in correct position");
    
    // ========================================================================
    // NEW TEST 9: Duplicate insertion position verification
    // ========================================================================
    std::vector<float> points9 = {0.2f, 0.4f, 0.6f};
    distlib::bucket_add_partition_point(points9, 0.4f);
    assert(points9.size() == 4 && "Should have 4 elements");
    assert(std::is_sorted(points9.begin(), points9.end()) && "Should remain sorted");
    // Verify 0.4 appears twice
    int count_04 = std::count_if(points9.begin(), points9.end(), 
        [](float x) { return std::abs(x - 0.4f) < 0.001f; });
    assert(count_04 == 2 && "Should have two 0.4 values");
    
    // ========================================================================
    // NEW TEST 10: Large vector stress test
    // ========================================================================
    std::vector<float> points10;
    for (int i = 0; i < 100; i++) {
        distlib::bucket_add_partition_point(points10, i * 0.01f);
    }
    assert(points10.size() == 100 && "Should have 100 elements");
    assert(std::is_sorted(points10.begin(), points10.end()) && "Should be sorted");
    // Verify first and last
    assert(std::abs(points10[0] - 0.0f) < 0.001f && "First should be 0.0");
    assert(std::abs(points10[99] - 0.99f) < 0.001f && "Last should be 0.99");
    
    std::cout << "All bucket_add_partition_point tests passed! (10 tests)" << std::endl;
    return 0;
}

// ============================================================================
// MAIN: Run all improved tests
// ============================================================================

int main() {
    std::cout << "Running improved distlib tests..." << std::endl;
    std::cout << "=================================" << std::endl;
    
    test_bucket_remove_partition_point_improved();
    test_bucket_less_than_improved();
    test_bucket_add_partition_point_improved();
    
    std::cout << "=================================" << std::endl;
    std::cout << "All improved tests passed!" << std::endl;
    std::cout << "Total new tests added: 10" << std::endl;
    std::cout << "  - bucket_remove_partition_point: +3 tests" << std::endl;
    std::cout << "  - bucket_less_than: +5 tests" << std::endl;
    std::cout << "  - bucket_add_partition_point: +2 tests" << std::endl;
    
    return 0;
}
