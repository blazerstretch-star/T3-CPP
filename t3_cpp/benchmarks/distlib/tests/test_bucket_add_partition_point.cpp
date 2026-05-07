#include "../src/distlib_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <algorithm>

int main() {
    constexpr float TOLERANCE = 0.001f;
    
    // Test 1: Add to middle - verify sorted order maintained
    std::vector<float> points1 = {0.2f, 0.4f, 0.6f, 0.8f};
    distlib::bucket_add_partition_point(points1, 0.5f);
    assert(points1.size() == 5 && "Should have 5 points");
    assert(std::is_sorted(points1.begin(), points1.end()) && "Should remain sorted");
    // Verify 0.5 is present and in correct position
    auto it = std::find_if(points1.begin(), points1.end(), 
        [](float x) { return std::abs(x - 0.5f) < 0.001f; });
    assert(it != points1.end() && "0.5 should be in vector");
    size_t idx = std::distance(points1.begin(), it);
    if (idx > 0) assert(points1[idx-1] < 0.5f && "Element before 0.5 should be smaller");
    if (idx < points1.size()-1) assert(points1[idx+1] > 0.5f && "Element after 0.5 should be larger");
    
    // Test 2: Add to beginning
    std::vector<float> points2 = {0.2f, 0.4f, 0.6f};
    distlib::bucket_add_partition_point(points2, 0.1f);
    assert(points2.size() == 4 && "Should have 4 points");
    assert(std::abs(points2[0] - 0.1f) < TOLERANCE && "0.1 should be at beginning");
    assert(std::is_sorted(points2.begin(), points2.end()) && "Should remain sorted");
    
    // Test 3: Add to end
    std::vector<float> points3 = {0.2f, 0.4f, 0.6f};
    distlib::bucket_add_partition_point(points3, 0.9f);
    assert(points3.size() == 4 && "Should have 4 points");
    assert(std::abs(points3.back() - 0.9f) < TOLERANCE && "0.9 should be at end");
    assert(std::is_sorted(points3.begin(), points3.end()) && "Should remain sorted");
    
    // Test 4: Add to empty vector
    std::vector<float> points4;
    distlib::bucket_add_partition_point(points4, 0.5f);
    assert(points4.size() == 1 && "Should have 1 point");
    assert(std::abs(points4[0] - 0.5f) < TOLERANCE && "Should contain 0.5");
    
    // Test 5: Add duplicate - verify it's inserted and sorted
    std::vector<float> points5 = {0.2f, 0.4f, 0.6f};
    distlib::bucket_add_partition_point(points5, 0.4f);
    assert(points5.size() == 4 && "Should allow duplicate");
    assert(std::is_sorted(points5.begin(), points5.end()) && "Should remain sorted");
    // Count occurrences of 0.4
    int count = std::count_if(points5.begin(), points5.end(),
        [](float x) { return std::abs(x - 0.4f) < 0.001f; });
    assert(count == 2 && "Should have two 0.4 values");
    
    // Test 6: Add to single element
    std::vector<float> points6 = {0.5f};
    distlib::bucket_add_partition_point(points6, 0.3f);
    assert(points6.size() == 2 && "Should have 2 elements");
    assert(std::is_sorted(points6.begin(), points6.end()) && "Should be sorted");
    assert(std::abs(points6[0] - 0.3f) < TOLERANCE && "First should be 0.3");
    assert(std::abs(points6[1] - 0.5f) < TOLERANCE && "Second should be 0.5");
    
    // Test 7: Add multiple in sequence - verify cumulative sorting
    std::vector<float> points7;
    distlib::bucket_add_partition_point(points7, 0.5f);
    distlib::bucket_add_partition_point(points7, 0.3f);
    distlib::bucket_add_partition_point(points7, 0.7f);
    distlib::bucket_add_partition_point(points7, 0.1f);
    assert(points7.size() == 4 && "Should have 4 elements");
    assert(std::is_sorted(points7.begin(), points7.end()) && "Should be sorted");
    assert(std::abs(points7[0] - 0.1f) < TOLERANCE && "First should be 0.1");
    assert(std::abs(points7[3] - 0.7f) < TOLERANCE && "Last should be 0.7");
    
    // Test 8: Verify insertion maintains order (not just sorting)
    std::vector<float> points8 = {0.1f, 0.3f, 0.5f, 0.7f, 0.9f};
    distlib::bucket_add_partition_point(points8, 0.6f);
    assert(points8.size() == 6 && "Should have 6 elements");
    assert(std::is_sorted(points8.begin(), points8.end()) && "Should remain sorted");
    // Verify 0.6 is between 0.5 and 0.7
    auto it8 = std::find_if(points8.begin(), points8.end(), 
        [](float x) { return std::abs(x - 0.6f) < 0.001f; });
    assert(it8 != points8.end() && "0.6 should be in vector");
    size_t idx8 = std::distance(points8.begin(), it8);
    assert(idx8 > 0 && idx8 < points8.size() - 1 && "0.6 should be in middle");
    assert(points8[idx8-1] < 0.6f && points8[idx8+1] > 0.6f && "0.6 in correct position");
    
    // Test 9: Duplicate insertion position verification
    std::vector<float> points9 = {0.2f, 0.4f, 0.6f};
    distlib::bucket_add_partition_point(points9, 0.4f);
    assert(points9.size() == 4 && "Should have 4 elements");
    assert(std::is_sorted(points9.begin(), points9.end()) && "Should remain sorted");
    // Both 0.4 values should be adjacent
    int count_04 = 0;
    for (size_t i = 0; i < points9.size(); ++i) {
        if (std::abs(points9[i] - 0.4f) < TOLERANCE) count_04++;
    }
    assert(count_04 == 2 && "Should have exactly two 0.4 values");
    
    // Test 10: Large vector stress test
    std::vector<float> points10;
    for (int i = 0; i < 100; i++) {
        distlib::bucket_add_partition_point(points10, i * 0.01f);
    }
    assert(points10.size() == 100 && "Should have 100 elements");
    assert(std::is_sorted(points10.begin(), points10.end()) && "Should be sorted");
    // Verify first and last
    assert(std::abs(points10[0] - 0.0f) < TOLERANCE && "First should be ~0.0");
    assert(std::abs(points10[99] - 0.99f) < TOLERANCE && "Last should be ~0.99");
    
    std::cout << "All bucket_add_partition_point tests passed! (10 tests)" << std::endl;
    return 0;
}
