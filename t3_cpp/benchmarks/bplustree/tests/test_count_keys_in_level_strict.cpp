#include "../src/bptree.h"
#include <cassert>
#include <iostream>
#include <vector>
#include <queue>
#include <numeric>

using namespace bptree;

int main() {
    // ========================================
    // PROPERTY 1: Non-negative result
    // ========================================
    auto check_non_negative = [](int result) {
        assert(result >= 0 && "Key count must be non-negative");
    };
    
    // ========================================
    // PROPERTY 2: Sum of all levels equals total keys in tree
    // ========================================
    auto check_sum_equals_total = [](Node* root) {
        if (root == NULL) return;
        
        // Count total keys in tree
        int total_keys = 0;
        std::vector<Node*> queue = {root};
        while (!queue.empty()) {
            Node* node = queue.back();
            queue.pop_back();
            total_keys += node->keys.size();
            
            if (!node->isLeaf) {
                for (Node* child : node->ptr2TreeOrData.ptr2Tree) {
                    if (child) queue.push_back(child);
                }
            }
        }
        
        // Sum keys across all levels
        int sum_by_level = 0;
        int level = 0;
        while (true) {
            int count = count_keys_in_level(root, level);
            if (count == 0) break;
            sum_by_level += count;
            level++;
        }
        
        assert(sum_by_level == total_keys && 
               "Sum of keys across all levels must equal total keys");
    };
    
    // ========================================
    // PROPERTY 3: Non-existent level returns 0
    // ========================================
    auto check_nonexistent_level = [](Node* root, int max_level) {
        for (int level = max_level + 1; level < max_level + 5; level++) {
            int count = count_keys_in_level(root, level);
            assert(count == 0 && "Non-existent level must return 0");
        }
    };
    
    // ========================================
    // PROPERTY 4: NULL root returns 0 for any level
    // ========================================
    auto check_null_safety = []() {
        assert(count_keys_in_level(NULL, 0) == 0 && "NULL root must return 0");
        assert(count_keys_in_level(NULL, 1) == 0 && "NULL root must return 0");
        assert(count_keys_in_level(NULL, 10) == 0 && "NULL root must return 0");
        assert(count_keys_in_level(NULL, -1) == 0 && "NULL root must return 0");
    };
    
    // ========================================
    // PROPERTY 5: Deterministic - same input gives same output
    // ========================================
    auto check_deterministic = [](Node* root, int level) {
        int result1 = count_keys_in_level(root, level);
        int result2 = count_keys_in_level(root, level);
        assert(result1 == result2 && "Function must be deterministic");
    };
    
    // ========================================
    // PROPERTY 6: Level 0 count equals root keys count
    // ========================================
    auto check_level_zero = [](Node* root) {
        if (root == NULL) return;
        int count = count_keys_in_level(root, 0);
        assert(count == (int)root->keys.size() && 
               "Level 0 count must equal root's key count");
    };
    
    // ========================================
    // PROPERTY 7: Verify count by manual BFS
    // ========================================
    auto verify_by_bfs = [](Node* root, int target_level) {
        if (root == NULL) {
            assert(count_keys_in_level(root, target_level) == 0);
            return;
        }
        
        std::queue<Node*> q;
        q.push(root);
        int current_level = 0;
        int expected_count = 0;
        
        while (!q.empty()) {
            int level_size = q.size();
            
            if (current_level == target_level) {
                for (int i = 0; i < level_size; i++) {
                    Node* node = q.front();
                    q.pop();
                    expected_count += node->keys.size();
                }
                break;
            }
            
            for (int i = 0; i < level_size; i++) {
                Node* node = q.front();
                q.pop();
                
                if (!node->isLeaf) {
                    for (Node* child : node->ptr2TreeOrData.ptr2Tree) {
                        if (child) q.push(child);
                    }
                }
            }
            current_level++;
        }
        
        int actual_count = count_keys_in_level(root, target_level);
        assert(actual_count == expected_count && 
               "Count must match BFS verification");
    };
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: NULL root
    check_null_safety();
    
    // Test 2: Single node (root is leaf)
    Node* root1 = new Node();
    root1->isLeaf = true;
    root1->keys = {5, 10, 15, 20};
    
    int count1_0 = count_keys_in_level(root1, 0);
    assert(count1_0 == 4 && "Level 0 should have 4 keys");
    check_non_negative(count1_0);
    check_level_zero(root1);
    check_deterministic(root1, 0);
    verify_by_bfs(root1, 0);
    
    int count1_1 = count_keys_in_level(root1, 1);
    assert(count1_1 == 0 && "Level 1 should have 0 keys");
    check_nonexistent_level(root1, 0);
    check_sum_equals_total(root1);
    
    // Test 3: Empty root
    Node* root2 = new Node();
    root2->isLeaf = true;
    root2->keys = {};
    
    assert(count_keys_in_level(root2, 0) == 0 && "Empty root has 0 keys");
    check_level_zero(root2);
    check_sum_equals_total(root2);
    
    // Test 4: Two-level tree
    Node* internal1 = new Node();
    internal1->isLeaf = false;
    internal1->keys = {20, 40};  // 2 keys at level 0
    new (&internal1->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* leaf1 = new Node();
    leaf1->isLeaf = true;
    leaf1->keys = {5, 10, 15};  // 3 keys
    
    Node* leaf2 = new Node();
    leaf2->isLeaf = true;
    leaf2->keys = {20, 25, 30, 35};  // 4 keys
    
    Node* leaf3 = new Node();
    leaf3->isLeaf = true;
    leaf3->keys = {40, 50};  // 2 keys
    
    internal1->ptr2TreeOrData.ptr2Tree.push_back(leaf1);
    internal1->ptr2TreeOrData.ptr2Tree.push_back(leaf2);
    internal1->ptr2TreeOrData.ptr2Tree.push_back(leaf3);
    
    int c0 = count_keys_in_level(internal1, 0);
    assert(c0 == 2 && "Level 0: 2 keys");
    check_level_zero(internal1);
    verify_by_bfs(internal1, 0);
    
    int c1 = count_keys_in_level(internal1, 1);
    assert(c1 == 9 && "Level 1: 3+4+2=9 keys");
    verify_by_bfs(internal1, 1);
    
    int c2 = count_keys_in_level(internal1, 2);
    assert(c2 == 0 && "Level 2: 0 keys");
    
    check_sum_equals_total(internal1);
    check_nonexistent_level(internal1, 1);
    check_deterministic(internal1, 0);
    check_deterministic(internal1, 1);
    
    // Test 5: Three-level tree
    Node* root3 = new Node();
    root3->isLeaf = false;
    root3->keys = {50, 100};  // 2 keys at level 0
    new (&root3->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* int1 = new Node();
    int1->isLeaf = false;
    int1->keys = {25};  // 1 key
    new (&int1->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* int2 = new Node();
    int2->isLeaf = false;
    int2->keys = {75};  // 1 key
    new (&int2->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* int3 = new Node();
    int3->isLeaf = false;
    int3->keys = {125, 150};  // 2 keys
    new (&int3->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    // Level 1 total: 1+1+2 = 4 keys
    
    Node* ll1 = new Node();
    ll1->isLeaf = true;
    ll1->keys = {10, 20};  // 2 keys
    
    Node* ll2 = new Node();
    ll2->isLeaf = true;
    ll2->keys = {25, 30, 40};  // 3 keys
    
    Node* ll3 = new Node();
    ll3->isLeaf = true;
    ll3->keys = {50, 60, 70};  // 3 keys
    
    Node* ll4 = new Node();
    ll4->isLeaf = true;
    ll4->keys = {75, 80, 90};  // 3 keys
    
    Node* ll5 = new Node();
    ll5->isLeaf = true;
    ll5->keys = {100, 110, 120};  // 3 keys
    
    Node* ll6 = new Node();
    ll6->isLeaf = true;
    ll6->keys = {125, 140};  // 2 keys
    
    Node* ll7 = new Node();
    ll7->isLeaf = true;
    ll7->keys = {150, 160, 170, 180};  // 4 keys
    
    // Level 2 total: 2+3+3+3+3+2+4 = 20 keys
    
    int1->ptr2TreeOrData.ptr2Tree.push_back(ll1);
    int1->ptr2TreeOrData.ptr2Tree.push_back(ll2);
    
    int2->ptr2TreeOrData.ptr2Tree.push_back(ll3);
    int2->ptr2TreeOrData.ptr2Tree.push_back(ll4);
    
    int3->ptr2TreeOrData.ptr2Tree.push_back(ll5);
    int3->ptr2TreeOrData.ptr2Tree.push_back(ll6);
    int3->ptr2TreeOrData.ptr2Tree.push_back(ll7);
    
    root3->ptr2TreeOrData.ptr2Tree.push_back(int1);
    root3->ptr2TreeOrData.ptr2Tree.push_back(int2);
    root3->ptr2TreeOrData.ptr2Tree.push_back(int3);
    
    int l0 = count_keys_in_level(root3, 0);
    assert(l0 == 2 && "Level 0: 2 keys");
    check_level_zero(root3);
    verify_by_bfs(root3, 0);
    
    int l1 = count_keys_in_level(root3, 1);
    assert(l1 == 4 && "Level 1: 1+1+2=4 keys");
    verify_by_bfs(root3, 1);
    
    int l2 = count_keys_in_level(root3, 2);
    assert(l2 == 20 && "Level 2: 2+3+3+3+3+2+4=20 keys");
    verify_by_bfs(root3, 2);
    
    int l3 = count_keys_in_level(root3, 3);
    assert(l3 == 0 && "Level 3: 0 keys");
    
    check_sum_equals_total(root3);
    check_nonexistent_level(root3, 2);
    check_deterministic(root3, 0);
    check_deterministic(root3, 1);
    check_deterministic(root3, 2);
    
    // Test 6: Wide tree (many nodes at one level)
    Node* wide_root = new Node();
    wide_root->isLeaf = false;
    new (&wide_root->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    for (int i = 0; i < 20; i++) {
        wide_root->keys.push_back(i * 10);
    }
    // Level 0: 20 keys
    
    int total_level1_keys = 0;
    for (int i = 0; i < 21; i++) {
        Node* wide_leaf = new Node();
        wide_leaf->isLeaf = true;
        int num_keys = (i % 3) + 1;  // 1, 2, or 3 keys per leaf
        for (int j = 0; j < num_keys; j++) {
            wide_leaf->keys.push_back(i * 10 + j);
        }
        total_level1_keys += num_keys;
        wide_root->ptr2TreeOrData.ptr2Tree.push_back(wide_leaf);
    }
    
    assert(count_keys_in_level(wide_root, 0) == 20);
    assert(count_keys_in_level(wide_root, 1) == total_level1_keys);
    check_sum_equals_total(wide_root);
    verify_by_bfs(wide_root, 0);
    verify_by_bfs(wide_root, 1);
    
    // Test 7: Deep tree (many levels)
    Node* deep_root = new Node();
    deep_root->isLeaf = false;
    deep_root->keys = {100};
    new (&deep_root->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* current = deep_root;
    for (int level = 1; level < 5; level++) {
        Node* next = new Node();
        next->isLeaf = false;
        next->keys = {level * 10};
        new (&next->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
        current->ptr2TreeOrData.ptr2Tree.push_back(next);
        current = next;
    }
    
    Node* deep_leaf = new Node();
    deep_leaf->isLeaf = true;
    deep_leaf->keys = {1, 2, 3};
    current->ptr2TreeOrData.ptr2Tree.push_back(deep_leaf);
    
    for (int level = 0; level < 5; level++) {
        int count = count_keys_in_level(deep_root, level);
        assert(count == 1 && "Each internal level has 1 key");
        verify_by_bfs(deep_root, level);
    }
    
    int leaf_level_count = count_keys_in_level(deep_root, 5);
    assert(leaf_level_count == 3 && "Leaf level has 3 keys");
    verify_by_bfs(deep_root, 5);
    
    check_sum_equals_total(deep_root);
    
    // Test 8: Unbalanced tree
    Node* unbal = new Node();
    unbal->isLeaf = false;
    unbal->keys = {50};
    new (&unbal->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* left_deep = new Node();
    left_deep->isLeaf = false;
    left_deep->keys = {25};
    new (&left_deep->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* right_shallow = new Node();
    right_shallow->isLeaf = true;
    right_shallow->keys = {75, 80};
    
    Node* left_leaf = new Node();
    left_leaf->isLeaf = true;
    left_leaf->keys = {10, 20};
    
    left_deep->ptr2TreeOrData.ptr2Tree.push_back(left_leaf);
    unbal->ptr2TreeOrData.ptr2Tree.push_back(left_deep);
    unbal->ptr2TreeOrData.ptr2Tree.push_back(right_shallow);
    
    assert(count_keys_in_level(unbal, 0) == 1);  // Root: 1 key
    assert(count_keys_in_level(unbal, 1) == 3);  // left_deep(1) + right_shallow(2) = 3
    assert(count_keys_in_level(unbal, 2) == 2);  // left_leaf: 2 keys
    check_sum_equals_total(unbal);
    
    // Test 9: Negative level (edge case)
    int neg_result = count_keys_in_level(root3, -1);
    assert(neg_result == 0 && "Negative level should return 0");
    
    // Test 10: Very large level number
    int large_result = count_keys_in_level(root3, 1000);
    assert(large_result == 0 && "Very large level should return 0");
    
    std::cout << "All count_keys_in_level strict tests passed!" << std::endl;
    return 0;
}
