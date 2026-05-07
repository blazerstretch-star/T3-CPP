#include "../src/bptree.h"
#include <cassert>
#include <iostream>
#include <vector>
#include <algorithm>
#include <climits>

using namespace bptree;

int main() {
    // ========================================
    // PROPERTY 1: Result maintains sorted order after insertion
    // ========================================
    auto check_sorted_after_insert = [](const std::vector<int>& keys, int key) {
        int pos = find_insert_position(keys, key);
        
        // Create copy and insert
        std::vector<int> result = keys;
        result.insert(result.begin() + pos, key);
        
        // Verify sorted
        for (size_t i = 1; i < result.size(); i++) {
            assert(result[i] >= result[i-1] && 
                   "Array must remain sorted after insertion at returned position");
        }
    };
    
    // ========================================
    // PROPERTY 2: Position is in valid range [0, size]
    // ========================================
    auto check_valid_range = [](const std::vector<int>& keys, int key) {
        int pos = find_insert_position(keys, key);
        assert(pos >= 0 && pos <= (int)keys.size() && 
               "Position must be in range [0, size]");
    };
    
    // ========================================
    // PROPERTY 3: Upper bound semantics - all elements before pos are <= key
    //             and all elements at/after pos are > key
    // ========================================
    auto check_upper_bound_semantics = [](const std::vector<int>& keys, int key) {
        int pos = find_insert_position(keys, key);
        
        // All elements before pos should be <= key
        for (int i = 0; i < pos; i++) {
            assert(keys[i] <= key && 
                   "All elements before position must be <= key");
        }
        
        // All elements at/after pos should be > key
        for (int i = pos; i < (int)keys.size(); i++) {
            assert(keys[i] > key && 
                   "All elements at/after position must be > key");
        }
    };
    
    // ========================================
    // PROPERTY 4: Deterministic - same input gives same output
    // ========================================
    auto check_deterministic = [](const std::vector<int>& keys, int key) {
        int pos1 = find_insert_position(keys, key);
        int pos2 = find_insert_position(keys, key);
        assert(pos1 == pos2 && "Function must be deterministic");
    };
    
    // ========================================
    // PROPERTY 5: Monotonicity - if key1 < key2, then pos1 <= pos2
    // ========================================
    auto check_monotonic = [](const std::vector<int>& keys, int key1, int key2) {
        if (key1 < key2) {
            int pos1 = find_insert_position(keys, key1);
            int pos2 = find_insert_position(keys, key2);
            assert(pos1 <= pos2 && 
                   "Smaller keys must have smaller or equal positions");
        }
    };
    
    // ========================================
    // PROPERTY 6: Empty vector returns 0
    // ========================================
    auto check_empty_invariant = []() {
        std::vector<int> empty;
        assert(find_insert_position(empty, 0) == 0 && "Empty vector must return 0");
        assert(find_insert_position(empty, -100) == 0 && "Empty vector must return 0");
        assert(find_insert_position(empty, 100) == 0 && "Empty vector must return 0");
    };
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: Empty vector
    check_empty_invariant();
    
    // Test 2: Single element - all positions
    std::vector<int> single = {10};
    
    int pos_before = find_insert_position(single, 5);
    check_valid_range(single, 5);
    check_sorted_after_insert(single, 5);
    check_upper_bound_semantics(single, 5);
    assert(pos_before == 0 && "Key < element should insert at 0");
    
    int pos_equal = find_insert_position(single, 10);
    check_valid_range(single, 10);
    check_sorted_after_insert(single, 10);
    check_upper_bound_semantics(single, 10);
    assert(pos_equal == 1 && "Key == element should insert after (upper_bound)");
    
    int pos_after = find_insert_position(single, 15);
    check_valid_range(single, 15);
    check_sorted_after_insert(single, 15);
    check_upper_bound_semantics(single, 15);
    assert(pos_after == 1 && "Key > element should insert at end");
    
    // Test 3: Multiple elements - insert at beginning
    std::vector<int> keys1 = {10, 20, 30, 40};
    
    int pos1 = find_insert_position(keys1, 5);
    check_valid_range(keys1, 5);
    check_sorted_after_insert(keys1, 5);
    check_upper_bound_semantics(keys1, 5);
    check_deterministic(keys1, 5);
    assert(pos1 == 0 && "Key < all elements should insert at 0");
    
    // Test 4: Insert in middle
    int pos2 = find_insert_position(keys1, 15);
    check_valid_range(keys1, 15);
    check_sorted_after_insert(keys1, 15);
    check_upper_bound_semantics(keys1, 15);
    check_deterministic(keys1, 15);
    assert(pos2 == 1 && "Key between elements should insert in middle");
    
    int pos3 = find_insert_position(keys1, 25);
    check_valid_range(keys1, 25);
    check_sorted_after_insert(keys1, 25);
    check_upper_bound_semantics(keys1, 25);
    assert(pos3 == 2);
    
    int pos4 = find_insert_position(keys1, 35);
    check_valid_range(keys1, 35);
    check_sorted_after_insert(keys1, 35);
    check_upper_bound_semantics(keys1, 35);
    assert(pos4 == 3);
    
    // Test 5: Insert at end
    int pos5 = find_insert_position(keys1, 50);
    check_valid_range(keys1, 50);
    check_sorted_after_insert(keys1, 50);
    check_upper_bound_semantics(keys1, 50);
    check_deterministic(keys1, 50);
    assert(pos5 == 4 && "Key > all elements should insert at end");
    
    // Test 6: Duplicate keys (upper_bound behavior)
    std::vector<int> keys2 = {10, 20, 20, 20, 30};
    
    int dup_pos = find_insert_position(keys2, 20);
    check_valid_range(keys2, 20);
    check_sorted_after_insert(keys2, 20);
    check_upper_bound_semantics(keys2, 20);
    // upper_bound returns position after all equal elements
    assert(dup_pos == 4 && "Duplicate key should insert after all equal elements");
    
    // Test 7: All same elements
    std::vector<int> all_same = {5, 5, 5, 5, 5};
    
    int same_pos1 = find_insert_position(all_same, 5);
    check_valid_range(all_same, 5);
    check_sorted_after_insert(all_same, 5);
    check_upper_bound_semantics(all_same, 5);
    assert(same_pos1 == 5 && "Should insert after all equal elements");
    
    int same_pos2 = find_insert_position(all_same, 3);
    check_valid_range(all_same, 3);
    check_sorted_after_insert(all_same, 3);
    assert(same_pos2 == 0);
    
    int same_pos3 = find_insert_position(all_same, 7);
    check_valid_range(all_same, 7);
    check_sorted_after_insert(all_same, 7);
    assert(same_pos3 == 5);
    
    // Test 8: Negative numbers
    std::vector<int> negative = {-50, -30, -10, 0, 10, 30};
    
    int neg_pos1 = find_insert_position(negative, -40);
    check_valid_range(negative, -40);
    check_sorted_after_insert(negative, -40);
    check_upper_bound_semantics(negative, -40);
    check_deterministic(negative, -40);
    
    int neg_pos2 = find_insert_position(negative, -100);
    check_valid_range(negative, -100);
    check_sorted_after_insert(negative, -100);
    assert(neg_pos2 == 0);
    
    int neg_pos3 = find_insert_position(negative, 0);
    check_valid_range(negative, 0);
    check_sorted_after_insert(negative, 0);
    check_upper_bound_semantics(negative, 0);
    
    // Test 9: Monotonicity property
    std::vector<int> mono = {10, 20, 30, 40, 50};
    check_monotonic(mono, 5, 15);
    check_monotonic(mono, 15, 25);
    check_monotonic(mono, 25, 35);
    check_monotonic(mono, 35, 45);
    check_monotonic(mono, 45, 55);
    
    // Test 10: Large array (stress test)
    std::vector<int> large;
    for (int i = 0; i < 1000; i++) {
        large.push_back(i * 2); // Even numbers 0, 2, 4, ..., 1998
    }
    
    // Test odd numbers (should insert between)
    for (int i = 0; i < 100; i++) {
        int odd = i * 2 + 1;
        check_valid_range(large, odd);
        check_sorted_after_insert(large, odd);
        check_upper_bound_semantics(large, odd);
        check_deterministic(large, odd);
    }
    
    // Test 11: Boundary values
    std::vector<int> boundary = {INT_MIN + 1, 0, INT_MAX - 1};
    
    int b1 = find_insert_position(boundary, INT_MIN);
    check_valid_range(boundary, INT_MIN);
    check_sorted_after_insert(boundary, INT_MIN);
    assert(b1 == 0);
    
    int b2 = find_insert_position(boundary, INT_MAX);
    check_valid_range(boundary, INT_MAX);
    check_sorted_after_insert(boundary, INT_MAX);
    assert(b2 == 3);
    
    // Test 12: Two elements (edge case)
    std::vector<int> two = {10, 20};
    
    assert(find_insert_position(two, 5) == 0);
    assert(find_insert_position(two, 15) == 1);
    assert(find_insert_position(two, 25) == 2);
    assert(find_insert_position(two, 10) == 1); // upper_bound
    assert(find_insert_position(two, 20) == 2); // upper_bound
    
    check_sorted_after_insert(two, 5);
    check_sorted_after_insert(two, 15);
    check_sorted_after_insert(two, 25);
    
    std::cout << "All find_insert_position strict tests passed!" << std::endl;
    return 0;
}
