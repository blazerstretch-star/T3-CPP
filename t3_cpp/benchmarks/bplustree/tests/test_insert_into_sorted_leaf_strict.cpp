#include "../src/bptree.h"
#include <cassert>
#include <iostream>
#include <vector>
#include <algorithm>
#include <climits>

using namespace bptree;

int main() {
    // ========================================
    // PROPERTY 1: Keys remain sorted after insertion
    // ========================================
    auto check_sorted = [](Node* leaf) {
        for (size_t i = 1; i < leaf->keys.size(); i++) {
            assert(leaf->keys[i] >= leaf->keys[i-1] && 
                   "Keys must remain sorted after insertion");
        }
    };
    
    // ========================================
    // PROPERTY 2: Size increases by exactly 1
    // ========================================
    auto check_size_increase = [](size_t before_size, Node* leaf) {
        assert(leaf->keys.size() == before_size + 1 && 
               "Size must increase by exactly 1");
        assert(leaf->ptr2TreeOrData.dataPtr.size() == before_size + 1 && 
               "Data pointer array size must match keys size");
    };
    
    // ========================================
    // PROPERTY 3: Keys and dataPtr arrays have same size
    // ========================================
    auto check_array_sync = [](Node* leaf) {
        assert(leaf->keys.size() == leaf->ptr2TreeOrData.dataPtr.size() && 
               "Keys and dataPtr arrays must have same size");
    };
    
    // ========================================
    // PROPERTY 4: Inserted key exists in array
    // ========================================
    auto check_key_exists = [](Node* leaf, int key) {
        bool found = false;
        for (int k : leaf->keys) {
            if (k == key) {
                found = true;
                break;
            }
        }
        assert(found && "Inserted key must exist in keys array");
    };
    
    // ========================================
    // PROPERTY 5: Inserted dataPtr exists at corresponding position
    // ========================================
    auto check_dataptr_exists = [](Node* leaf, int key, FILE* filePtr) {
        for (size_t i = 0; i < leaf->keys.size(); i++) {
            if (leaf->keys[i] == key) {
                assert(leaf->ptr2TreeOrData.dataPtr[i] == filePtr && 
                       "DataPtr must be at same index as key");
                return;
            }
        }
        assert(false && "Key not found after insertion");
    };
    
    // ========================================
    // PROPERTY 6: NULL leaf is handled safely (no crash)
    // ========================================
    auto check_null_safety = []() {
        // Should not crash
        insert_into_sorted_leaf(NULL, 10, NULL);
        // If we reach here, NULL was handled safely
    };
    
    // ========================================
    // PROPERTY 7: Original keys are preserved
    // ========================================
    auto check_keys_preserved = [](const std::vector<int>& original_keys, 
                                     Node* leaf, int new_key) {
        for (int orig_key : original_keys) {
            bool found = false;
            for (int k : leaf->keys) {
                if (k == orig_key) {
                    found = true;
                    break;
                }
            }
            assert(found && "Original keys must be preserved");
        }
    };
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: NULL leaf safety
    check_null_safety();
    
    // Test 2: Insert into empty leaf
    Node* leaf1 = new Node();
    leaf1->isLeaf = true;
    new (&leaf1->ptr2TreeOrData.dataPtr) std::vector<FILE*>;
    
    size_t size_before = leaf1->keys.size();
    insert_into_sorted_leaf(leaf1, 10, NULL);
    
    check_size_increase(size_before, leaf1);
    check_sorted(leaf1);
    check_array_sync(leaf1);
    check_key_exists(leaf1, 10);
    check_dataptr_exists(leaf1, 10, NULL);
    assert(leaf1->keys[0] == 10 && "First insertion should be at index 0");
    
    // Test 3: Insert at beginning (smaller than all)
    Node* leaf2 = new Node();
    leaf2->isLeaf = true;
    new (&leaf2->ptr2TreeOrData.dataPtr) std::vector<FILE*>;
    leaf2->keys = {10, 20, 30};
    leaf2->ptr2TreeOrData.dataPtr = {NULL, NULL, NULL};
    
    std::vector<int> original2 = leaf2->keys;
    size_before = leaf2->keys.size();
    insert_into_sorted_leaf(leaf2, 5, NULL);
    
    check_size_increase(size_before, leaf2);
    check_sorted(leaf2);
    check_array_sync(leaf2);
    check_key_exists(leaf2, 5);
    check_keys_preserved(original2, leaf2, 5);
    assert(leaf2->keys[0] == 5 && "Smallest key should be at beginning");
    
    // Test 4: Insert at end (larger than all)
    Node* leaf3 = new Node();
    leaf3->isLeaf = true;
    new (&leaf3->ptr2TreeOrData.dataPtr) std::vector<FILE*>;
    leaf3->keys = {10, 20, 30};
    leaf3->ptr2TreeOrData.dataPtr = {NULL, NULL, NULL};
    
    std::vector<int> original3 = leaf3->keys;
    size_before = leaf3->keys.size();
    insert_into_sorted_leaf(leaf3, 40, NULL);
    
    check_size_increase(size_before, leaf3);
    check_sorted(leaf3);
    check_array_sync(leaf3);
    check_key_exists(leaf3, 40);
    check_keys_preserved(original3, leaf3, 40);
    assert(leaf3->keys[3] == 40 && "Largest key should be at end");
    
    // Test 5: Insert in middle
    Node* leaf4 = new Node();
    leaf4->isLeaf = true;
    new (&leaf4->ptr2TreeOrData.dataPtr) std::vector<FILE*>;
    leaf4->keys = {10, 20, 30, 40};
    leaf4->ptr2TreeOrData.dataPtr = {NULL, NULL, NULL, NULL};
    
    std::vector<int> original4 = leaf4->keys;
    size_before = leaf4->keys.size();
    insert_into_sorted_leaf(leaf4, 25, NULL);
    
    check_size_increase(size_before, leaf4);
    check_sorted(leaf4);
    check_array_sync(leaf4);
    check_key_exists(leaf4, 25);
    check_keys_preserved(original4, leaf4, 25);
    
    // Verify exact position
    bool found_25 = false;
    for (size_t i = 0; i < leaf4->keys.size(); i++) {
        if (leaf4->keys[i] == 25) {
            assert(i > 0 && leaf4->keys[i-1] < 25 && 
                   "Key before 25 must be smaller");
            assert(i < leaf4->keys.size() - 1 && leaf4->keys[i+1] > 25 && 
                   "Key after 25 must be larger");
            found_25 = true;
            break;
        }
    }
    assert(found_25);
    
    // Test 6: Multiple insertions in sequence
    Node* leaf5 = new Node();
    leaf5->isLeaf = true;
    new (&leaf5->ptr2TreeOrData.dataPtr) std::vector<FILE*>;
    
    std::vector<int> insert_sequence = {50, 20, 80, 10, 60, 30, 70, 40};
    
    for (int key : insert_sequence) {
        size_before = leaf5->keys.size();
        insert_into_sorted_leaf(leaf5, key, NULL);
        check_size_increase(size_before, leaf5);
        check_sorted(leaf5);
        check_array_sync(leaf5);
        check_key_exists(leaf5, key);
    }
    
    // Final array should be sorted
    assert(leaf5->keys.size() == insert_sequence.size());
    check_sorted(leaf5);
    
    // Test 7: Duplicate keys
    Node* leaf6 = new Node();
    leaf6->isLeaf = true;
    new (&leaf6->ptr2TreeOrData.dataPtr) std::vector<FILE*>;
    leaf6->keys = {10, 20, 30};
    leaf6->ptr2TreeOrData.dataPtr = {NULL, NULL, NULL};
    
    size_before = leaf6->keys.size();
    insert_into_sorted_leaf(leaf6, 20, NULL);
    
    check_size_increase(size_before, leaf6);
    check_sorted(leaf6);
    check_array_sync(leaf6);
    
    // Count occurrences of 20
    int count_20 = 0;
    for (int k : leaf6->keys) {
        if (k == 20) count_20++;
    }
    assert(count_20 == 2 && "Duplicate key should be inserted");
    
    // Test 8: Negative numbers
    Node* leaf7 = new Node();
    leaf7->isLeaf = true;
    new (&leaf7->ptr2TreeOrData.dataPtr) std::vector<FILE*>;
    leaf7->keys = {-50, -20, 0, 20, 50};
    leaf7->ptr2TreeOrData.dataPtr = {NULL, NULL, NULL, NULL, NULL};
    
    std::vector<int> original7 = leaf7->keys;
    insert_into_sorted_leaf(leaf7, -30, NULL);
    
    check_sorted(leaf7);
    check_array_sync(leaf7);
    check_key_exists(leaf7, -30);
    check_keys_preserved(original7, leaf7, -30);
    
    // Test 9: DataPtr tracking with non-NULL pointers
    Node* leaf8 = new Node();
    leaf8->isLeaf = true;
    new (&leaf8->ptr2TreeOrData.dataPtr) std::vector<FILE*>;
    
    FILE* ptr1 = (FILE*)0x1000; // Mock pointer
    FILE* ptr2 = (FILE*)0x2000;
    FILE* ptr3 = (FILE*)0x3000;
    
    insert_into_sorted_leaf(leaf8, 20, ptr2);
    insert_into_sorted_leaf(leaf8, 10, ptr1);
    insert_into_sorted_leaf(leaf8, 30, ptr3);
    
    check_sorted(leaf8);
    check_array_sync(leaf8);
    
    // Verify dataPtr correspondence
    for (size_t i = 0; i < leaf8->keys.size(); i++) {
        if (leaf8->keys[i] == 10) {
            assert(leaf8->ptr2TreeOrData.dataPtr[i] == ptr1);
        } else if (leaf8->keys[i] == 20) {
            assert(leaf8->ptr2TreeOrData.dataPtr[i] == ptr2);
        } else if (leaf8->keys[i] == 30) {
            assert(leaf8->ptr2TreeOrData.dataPtr[i] == ptr3);
        }
    }
    
    // Test 10: Large number of insertions (stress test)
    Node* leaf9 = new Node();
    leaf9->isLeaf = true;
    new (&leaf9->ptr2TreeOrData.dataPtr) std::vector<FILE*>;
    
    // Insert 100 random-order keys
    std::vector<int> random_keys;
    for (int i = 0; i < 100; i++) {
        random_keys.push_back(i);
    }
    // Shuffle (simple reverse for determinism)
    std::reverse(random_keys.begin(), random_keys.end());
    
    for (int key : random_keys) {
        size_before = leaf9->keys.size();
        insert_into_sorted_leaf(leaf9, key, NULL);
        check_size_increase(size_before, leaf9);
        check_sorted(leaf9);
        check_array_sync(leaf9);
    }
    
    assert(leaf9->keys.size() == 100);
    check_sorted(leaf9);
    
    // Test 11: Boundary values
    Node* leaf10 = new Node();
    leaf10->isLeaf = true;
    new (&leaf10->ptr2TreeOrData.dataPtr) std::vector<FILE*>;
    leaf10->keys = {0};
    leaf10->ptr2TreeOrData.dataPtr = {NULL};
    
    insert_into_sorted_leaf(leaf10, INT_MIN, NULL);
    check_sorted(leaf10);
    check_array_sync(leaf10);
    check_key_exists(leaf10, INT_MIN);
    
    insert_into_sorted_leaf(leaf10, INT_MAX, NULL);
    check_sorted(leaf10);
    check_array_sync(leaf10);
    check_key_exists(leaf10, INT_MAX);
    
    // Test 12: Single element, insert equal
    Node* leaf11 = new Node();
    leaf11->isLeaf = true;
    new (&leaf11->ptr2TreeOrData.dataPtr) std::vector<FILE*>;
    leaf11->keys = {10};
    leaf11->ptr2TreeOrData.dataPtr = {NULL};
    
    insert_into_sorted_leaf(leaf11, 10, NULL);
    check_sorted(leaf11);
    check_array_sync(leaf11);
    assert(leaf11->keys.size() == 2);
    
    std::cout << "All insert_into_sorted_leaf strict tests passed!" << std::endl;
    return 0;
}
