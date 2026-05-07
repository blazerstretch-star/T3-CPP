#include "../src/distlib_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    // Test 1: Create balanced view with 3 buckets, 5 partitions each
    std::vector<BucketPtr> buckets1;
    auto map1 = distlib::make_balanced_view_map(3, 5, buckets1);
    
    assert(buckets1.size() == 3 && "Should create 3 buckets");
    assert(map1->size() == 15 && "Should have 15 partition points total");
    
    // Verify each bucket has 5 partitions
    for (const auto& b : buckets1) {
        assert(b->partitions() == 5 && "Each bucket should have 5 partitions");
    }
    
    // Test 2: Verify balanced distribution (even spacing)
    float expected_delta = 1.0f / (3 * 5);
    std::vector<float> all_points;
    for (const auto& [point, bucket] : *map1) {
        all_points.push_back(point);
    }
    for (size_t i = 1; i < all_points.size(); i++) {
        float spacing = all_points[i] - all_points[i-1];
        assert(std::abs(spacing - expected_delta) < 0.001f && "Points should be evenly spaced");
    }
    
    // Test 3: Verify bucket naming
    assert(buckets1[0]->name() == "bucket-0" && "First bucket name");
    assert(buckets1[1]->name() == "bucket-1" && "Second bucket name");
    assert(buckets1[2]->name() == "bucket-2" && "Third bucket name");
    
    // Test 4: Verify partition points are sorted within each bucket
    for (const auto& b : buckets1) {
        for (int i = 1; i < b->partitions(); i++) {
            assert(b->partition_point(i) > b->partition_point(i-1) && "Partitions should be sorted");
        }
    }
    
    // Test 5: Verify map correctness (partition points map to correct buckets)
    for (const auto& b : buckets1) {
        for (int i = 0; i < b->partitions(); i++) {
            float point = b->partition_point(i);
            assert((*map1)[point] == b && "Partition should map to its bucket");
        }
    }
    
    // Test 6: Create with different parameters
    std::vector<BucketPtr> buckets2;
    auto map2 = distlib::make_balanced_view_map(2, 3, buckets2);
    
    assert(buckets2.size() == 2 && "Should create 2 buckets");
    assert(map2->size() == 6 && "Should have 6 partition points total");
    
    // Test 7: Verify interleaving pattern (bucket-0, bucket-1, bucket-0, bucket-1, ...)
    std::vector<BucketPtr> buckets3;
    auto map3 = distlib::make_balanced_view_map(2, 2, buckets3);
    std::vector<BucketPtr> order;
    for (const auto& [point, bucket] : *map3) {
        order.push_back(bucket);
    }
    assert(order[0] == buckets3[0] && "First partition to bucket-0");
    assert(order[1] == buckets3[1] && "Second partition to bucket-1");
    assert(order[2] == buckets3[0] && "Third partition to bucket-0");
    assert(order[3] == buckets3[1] && "Fourth partition to bucket-1");
    
    // Test 8: Single bucket, single partition
    std::vector<BucketPtr> buckets4;
    auto map4 = distlib::make_balanced_view_map(1, 1, buckets4);
    assert(buckets4.size() == 1 && map4->size() == 1);
    assert(std::abs(buckets4[0]->partition_point(0) - 1.0f) < 0.001f && "Single partition at 1.0");
    
    // Test 9: Invalid parameters (zero buckets)
    try {
        std::vector<BucketPtr> buckets5;
        distlib::make_balanced_view_map(0, 5, buckets5);
        assert(false && "Should throw exception for zero buckets");
    } catch (const std::invalid_argument&) {
        // Expected
    }
    
    // Test 10: Invalid parameters (zero partitions)
    try {
        std::vector<BucketPtr> buckets6;
        distlib::make_balanced_view_map(3, 0, buckets6);
        assert(false && "Should throw exception for zero partitions");
    } catch (const std::invalid_argument&) {
        // Expected
    }
    
    // Test 11: Invalid parameters (negative buckets)
    try {
        std::vector<BucketPtr> buckets7;
        distlib::make_balanced_view_map(-1, 5, buckets7);
        assert(false && "Should throw exception for negative buckets");
    } catch (const std::invalid_argument&) {
        // Expected
    }
    
    std::cout << "All make_balanced_view_map tests passed! (11 tests)" << std::endl;
    return 0;
}
