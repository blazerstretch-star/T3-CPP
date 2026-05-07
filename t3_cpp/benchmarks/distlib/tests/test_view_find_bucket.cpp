#include "../src/distlib_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    // Create test buckets
    auto b1 = std::make_shared<Bucket>("bucket1", std::vector<float>{0.2f, 0.5f});
    auto b2 = std::make_shared<Bucket>("bucket2", std::vector<float>{0.4f, 0.7f});
    auto b3 = std::make_shared<Bucket>("bucket3", std::vector<float>{0.6f, 0.9f});
    
    // Create partition map
    std::map<float, BucketPtr> partition_map;
    partition_map[0.2f] = b1;
    partition_map[0.4f] = b2;
    partition_map[0.5f] = b1;
    partition_map[0.6f] = b3;
    partition_map[0.7f] = b2;
    partition_map[0.9f] = b3;
    
    // Test 1: Find bucket for hash 0.3
    auto found1 = distlib::view_find_bucket(partition_map, 0.3f);
    assert(found1 == b2 && "Hash 0.3 should map to bucket2");
    
    // Test 2: Find bucket for hash 0.55
    auto found2 = distlib::view_find_bucket(partition_map, 0.55f);
    assert(found2 == b3 && "Hash 0.55 should map to bucket3");
    
    // Test 3: Find bucket for hash 0.95 (wraps around)
    auto found3 = distlib::view_find_bucket(partition_map, 0.95f);
    assert(found3 == b1 && "Hash 0.95 should wrap to bucket1");
    
    // Test 4: Invalid hash (negative)
    try {
        distlib::view_find_bucket(partition_map, -0.1f);
        assert(false && "Should throw exception for negative hash");
    } catch (const std::invalid_argument&) {
        // Expected
    }
    
    // Test 5: Invalid hash (too large)
    try {
        distlib::view_find_bucket(partition_map, 1.5f);
        assert(false && "Should throw exception for hash > 1");
    } catch (const std::invalid_argument&) {
        // Expected
    }
    
    // Test 6: Empty map should throw
    std::map<float, BucketPtr> empty_map;
    try {
        distlib::view_find_bucket(empty_map, 0.5f);
        assert(false && "Should throw for empty map");
    } catch (const std::invalid_argument&) {
        // Expected
    }
    
    // Test 7: Boundary value 0.0
    auto found4 = distlib::view_find_bucket(partition_map, 0.0f);
    assert(found4 == b1 && "Hash 0.0 should map to first bucket");
    
    // Test 8: Boundary value 1.0
    auto found5 = distlib::view_find_bucket(partition_map, 1.0f);
    assert(found5 == b1 && "Hash 1.0 should wrap to first bucket");
    
    // Test 9: Single bucket map
    std::map<float, BucketPtr> single_map;
    single_map[0.5f] = b1;
    auto found6 = distlib::view_find_bucket(single_map, 0.3f);
    assert(found6 == b1 && "Single bucket: before partition");
    auto found7 = distlib::view_find_bucket(single_map, 0.7f);
    assert(found7 == b1 && "Single bucket: after partition wraps");
    
    // Test 10: Hash exactly on partition point (upper_bound behavior)
    auto found8 = distlib::view_find_bucket(partition_map, 0.5f);
    assert(found8 == b3 && "Exact match uses upper_bound");
    
    std::cout << "All view_find_bucket tests passed! (10 tests)" << std::endl;
    return 0;
}
