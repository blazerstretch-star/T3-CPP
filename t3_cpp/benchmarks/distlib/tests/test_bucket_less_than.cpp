#include "../src/distlib_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>
#include <algorithm>

int main() {
    constexpr float TOLERANCE = 0.001f;
    constexpr float EPSILON = 1e-6f;  // kEpsilon from implementation
    
    std::vector<float> points1 = {0.3f, 0.6f, 0.9f};
    std::vector<float> points2 = {0.3f, 0.6f, 0.8f};
    
    // Test 1: Different names - lexicographic order
    bool lt1 = distlib::bucket_less_than("aaa", points1, "bbb", points1);
    assert(lt1 && "aaa should be less than bbb");
    
    bool lt2 = distlib::bucket_less_than("bbb", points1, "aaa", points1);
    assert(!lt2 && "bbb should not be less than aaa");
    
    // Test 2: Same name, different points
    bool lt3 = distlib::bucket_less_than("bucket", points2, "bucket", points1);
    assert(lt3 && "points2 should be less than points1 (0.8 < 0.9)");
    
    // Test 3: Transitivity property (a < b && b < c => a < c)
    std::vector<float> pa = {0.1f};
    std::vector<float> pb = {0.2f};
    std::vector<float> pc = {0.3f};
    bool a_lt_b = distlib::bucket_less_than("x", pa, "x", pb);
    bool b_lt_c = distlib::bucket_less_than("x", pb, "x", pc);
    bool a_lt_c = distlib::bucket_less_than("x", pa, "x", pc);
    assert(a_lt_b && b_lt_c && a_lt_c && "Transitivity: a<b && b<c => a<c");
    
    // Test 4: Reflexivity - a < a should always be false
    std::vector<float> p_reflex = {0.5f, 0.7f};
    bool reflexive = distlib::bucket_less_than("test", p_reflex, "test", p_reflex);
    assert(!reflexive && "Reflexivity: a < a should always be false");
    
    // Test 5: Epsilon tolerance - within epsilon should be treated as equal
    std::vector<float> p1 = {0.1f};
    std::vector<float> p2 = {0.1f + EPSILON * 0.5f};  // Within epsilon
    bool within_epsilon = distlib::bucket_less_than("a", p1, "a", p2);
    assert(!within_epsilon && "Within epsilon should be treated as equal (not less than)");
    
    // Test 6: Outside epsilon - should compare as different
    std::vector<float> p3 = {0.1f};
    std::vector<float> p4 = {0.1f + EPSILON * 10.0f};  // Outside epsilon
    bool outside_epsilon = distlib::bucket_less_than("a", p3, "a", p4);
    assert(outside_epsilon && "Outside epsilon should compare as less than");
    
    // Test 7: Empty vectors - should not be less than each other
    std::vector<float> empty1, empty2;
    bool empty_cmp = distlib::bucket_less_than("a", empty1, "a", empty2);
    assert(!empty_cmp && "Empty vectors should not be less than each other");
    
    // Test 8: Different length vectors - compare up to min length
    std::vector<float> short_vec = {0.5f};
    std::vector<float> long_vec = {0.5f, 0.7f};
    // With equal prefix, shorter should not be less than longer
    bool short_long = distlib::bucket_less_than("a", short_vec, "a", long_vec);
    assert(!short_long && "Equal prefix: shorter not less than longer");
    
    // Test 9: Antisymmetry - if a < b, then !(b < a)
    std::vector<float> p_anti1 = {0.3f};
    std::vector<float> p_anti2 = {0.5f};
    bool anti_ab = distlib::bucket_less_than("x", p_anti1, "x", p_anti2);
    bool anti_ba = distlib::bucket_less_than("x", p_anti2, "x", p_anti1);
    if (anti_ab) {
        assert(!anti_ba && "Antisymmetry: if a < b, then !(b < a)");
    }
    
    // Test 10: Name takes precedence over points
    std::vector<float> small_points = {0.1f};
    std::vector<float> large_points = {0.9f};
    bool name_precedence = distlib::bucket_less_than("aaa", large_points, "bbb", small_points);
    assert(name_precedence && "Name comparison takes precedence over points");
    
    std::cout << "All bucket_less_than tests passed! (10 tests)" << std::endl;
    return 0;
}
