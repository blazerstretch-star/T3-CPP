#include "../src/distlib_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    constexpr float TOLERANCE = 0.001f;
    std::vector<float> points = {0.2f, 0.4f, 0.6f, 0.8f};
    
    // Test 1: Point less than all partition points - should return first
    auto result1 = distlib::bucket_partition_point(points, 0.1f);
    assert(result1.first == 0 && "Index should be 0 for point less than all");
    assert(std::abs(result1.second - 0.2f) < TOLERANCE && "Should return first point");
    
    // Test 2: Point between partition points - should return next higher
    auto result2 = distlib::bucket_partition_point(points, 0.5f);
    assert(result2.first == 2 && "Index should be 2 for point between 0.4 and 0.6");
    assert(std::abs(result2.second - 0.6f) < TOLERANCE && "Should return 0.6");
    
    // Test 3: Point greater than all partition points - should wrap to first
    auto result3 = distlib::bucket_partition_point(points, 0.9f);
    assert(result3.first == 0 && "Should wrap to index 0 for point > all");
    assert(std::abs(result3.second - 0.2f) < TOLERANCE && "Should wrap to first point");
    
    // Test 4: Point exactly on partition point - upper_bound behavior (next point)
    auto result4 = distlib::bucket_partition_point(points, 0.4f);
    assert(result4.first == 2 && "Should return next point after 0.4 (upper_bound)");
    assert(std::abs(result4.second - 0.6f) < TOLERANCE && "Should return 0.6");
    
    // Test 5: Single element vector - point before
    std::vector<float> single = {0.5f};
    auto result5 = distlib::bucket_partition_point(single, 0.3f);
    assert(result5.first == 0 && "Single element: point before should return index 0");
    assert(std::abs(result5.second - 0.5f) < TOLERANCE && "Should return the single element");
    
    // Test 6: Single element vector - point after (wraparound)
    auto result6 = distlib::bucket_partition_point(single, 0.7f);
    assert(result6.first == 0 && "Single element: point after should wrap to index 0");
    assert(std::abs(result6.second - 0.5f) < TOLERANCE && "Should return the single element");
    
    // Test 7: Boundary value at 0.0
    auto result7 = distlib::bucket_partition_point(points, 0.0f);
    assert(result7.first == 0 && "Boundary 0.0 should return first point");
    assert(std::abs(result7.second - 0.2f) < TOLERANCE && "Should return 0.2");
    
    // Test 8: Boundary value at 1.0 (wraparound)
    auto result8 = distlib::bucket_partition_point(points, 1.0f);
    assert(result8.first == 0 && "Boundary 1.0 should wrap to first point");
    assert(std::abs(result8.second - 0.2f) < TOLERANCE && "Should wrap to 0.2");
    
    // Test 9: Floating-point precision - just after a point
    auto result9 = distlib::bucket_partition_point(points, 0.4f + 0.0001f);
    assert(result9.first == 2 && "Just after 0.4 should go to next point");
    assert(std::abs(result9.second - 0.6f) < TOLERANCE && "Should return 0.6");
    
    // Test 10: Two element vector
    std::vector<float> two = {0.3f, 0.7f};
    auto result10 = distlib::bucket_partition_point(two, 0.5f);
    assert(result10.first == 1 && "Should return index 1");
    assert(std::abs(result10.second - 0.7f) < TOLERANCE && "Should return 0.7");
    
    // Test 11: Verify wraparound consistency
    auto result11a = distlib::bucket_partition_point(points, 0.85f);
    auto result11b = distlib::bucket_partition_point(points, 0.95f);
    assert(result11a.first == 0 && result11b.first == 0 && "Both should wrap");
    assert(std::abs(result11a.second - result11b.second) < TOLERANCE && "Wraparound should be consistent");
    
    // Test 12: Multiple points in sequence
    std::vector<float> many = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f};
    auto result12 = distlib::bucket_partition_point(many, 0.35f);
    assert(result12.first == 3 && "Should find correct position in larger vector");
    assert(std::abs(result12.second - 0.4f) < TOLERANCE && "Should return 0.4");
    
    std::cout << "All bucket_partition_point tests passed! (12 tests)" << std::endl;
    return 0;
}
