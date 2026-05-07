#include "../src/distlib_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>
#include <algorithm>

int main() {
    constexpr float TOLERANCE = 0.001f;
    constexpr float EPSILON = 1e-6f;  // kEpsilon from implementation
    
    std::vector<float> points1 = {0.3f, 0.6f, 0.9f};
    std::vector<float> points2 = {0.3f, 0.6f, 0.9f};
    std::vector<float> points3 = {0.3f, 0.6f, 0.8f};
    
    // Test 1: Reflexivity - equal buckets should be equal
    bool eq1 = distlib::bucket_equals("bucket1", points1, "bucket1", points2);
    assert(eq1 && "Reflexivity: same name and points should be equal");
    
    // Test 2: Different points - should not be equal
    bool eq2 = distlib::bucket_equals("bucket", points1, "bucket", points3);
    assert(!eq2 && "Different points should not be equal");
    
    // Test 3: Epsilon tolerance - within epsilon should be equal
    std::vector<float> p1 = {0.1f, 0.2f};
    std::vector<float> p2 = {0.1f + EPSILON * 0.5f, 0.2f};
    assert(distlib::bucket_equals("a", p1, "a", p2) && "Within epsilon should be equal");
    
    // Test 4: Outside epsilon - should not be equal
    std::vector<float> p3 = {0.1f, 0.2f};
    std::vector<float> p4 = {0.1f + EPSILON * 10.0f, 0.2f};
    assert(!distlib::bucket_equals("a", p3, "a", p4) && "Outside epsilon should not be equal");
    
    // Test 5: Empty vectors - should be equal
    std::vector<float> empty1, empty2;
    assert(distlib::bucket_equals("a", empty1, "a", empty2) && "Empty vectors should be equal");
    
    // Test 6: Symmetry property - a == b implies b == a
    bool eq_ab = distlib::bucket_equals("x", points1, "x", points2);
    bool eq_ba = distlib::bucket_equals("x", points2, "x", points1);
    assert(eq_ab == eq_ba && "Symmetry: a == b should equal b == a");
    
    // Test 7: Different names - should not be equal even with same points
    bool eq_diff_names = distlib::bucket_equals("bucket1", points1, "bucket2", points1);
    assert(!eq_diff_names && "Different names should not be equal");
    
    // Test 8: Transitivity - if a == b and b == c, then a == c
    std::vector<float> pa = {0.5f};
    std::vector<float> pb = {0.5f + EPSILON * 0.3f};
    std::vector<float> pc = {0.5f + EPSILON * 0.4f};
    bool ab = distlib::bucket_equals("x", pa, "x", pb);
    bool bc = distlib::bucket_equals("x", pb, "x", pc);
    bool ac = distlib::bucket_equals("x", pa, "x", pc);
    // If ab and bc are both true, ac should also be true (transitivity)
    if (ab && bc) {
        assert(ac && "Transitivity: a==b && b==c => a==c");
    }
    
    // Test 9: Different lengths - behavior depends on implementation
    // Some implementations may consider equal prefix as equal, others may not
    std::vector<float> short_vec = {0.5f};
    std::vector<float> long_vec = {0.5f, 0.7f};
    bool diff_length = distlib::bucket_equals("a", short_vec, "a", long_vec);
    // Verify consistency: if equal, both less_than should be false
    bool lt_short_long = distlib::bucket_less_than("a", short_vec, "a", long_vec);
    bool lt_long_short = distlib::bucket_less_than("a", long_vec, "a", short_vec);
    if (diff_length) {
        assert(!lt_short_long && !lt_long_short && "If equal, neither should be less than");
    }
    
    // Test 10: Consistency with less_than - if equal, neither should be less
    std::vector<float> p_eq1 = {0.4f, 0.6f};
    std::vector<float> p_eq2 = {0.4f, 0.6f};
    bool is_equal = distlib::bucket_equals("test", p_eq1, "test", p_eq2);
    bool lt_eq1 = distlib::bucket_less_than("test", p_eq1, "test", p_eq2);
    bool lt_eq2 = distlib::bucket_less_than("test", p_eq2, "test", p_eq1);
    if (is_equal) {
        assert(!lt_eq1 && !lt_eq2 && "If equal, neither should be less than the other");
    }
    
    std::cout << "All bucket_equals tests passed! (10 tests)" << std::endl;
    return 0;
}
