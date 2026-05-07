#include "../src/bptree.h"
#include <cassert>
#include <iostream>
#include <vector>
#include <algorithm>
#include <climits>

using namespace bptree;

int main() {
    // ========================================
    // PROPERTY 1: Result must be a leaf node (if not NULL)
    // ========================================
    auto check_is_leaf = [](Node* result) {
        if (result != NULL) {
            assert(result->isLeaf == true && 
                   "Returned node must be a leaf");
        }
    };
    
    // ========================================
    // PROPERTY 2: Result must be reachable from root
    // ========================================
    auto check_reachable = [](Node* root, Node* result) {
        if (result == NULL) return;
        if (root == result) return; // Root is leaf
        
        bool found = false;
        std::vector<Node*> queue = {root};
        while (!queue.empty() && !found) {
            Node* node = queue.back();
            queue.pop_back();
            
            if (node == result) {
                found = true;
                break;
            }
            
            if (!node->isLeaf) {
                for (Node* child : node->ptr2TreeOrData.ptr2Tree) {
                    if (child) queue.push_back(child);
                }
            }
        }
        assert(found && "Result must be reachable from root");
    };
    
    // ========================================
    // PROPERTY 3: Key range consistency - if leaf has keys, 
    //             search key should fall within navigable range
    // ========================================
    auto check_key_range_consistency = [](Node* root, int key, Node* result) {
        if (result == NULL || result->keys.empty()) return;
        
        // For a proper B+ tree, the leaf found should be the correct one
        // We verify by checking that if we search for any key in result,
        // we get the same result
        if (!result->keys.empty()) {
            Node* verify = find_leaf_node(root, result->keys[0]);
            assert(verify == result && 
                   "Leaf node must be consistent for keys in its range");
        }
    };
    
    // ========================================
    // PROPERTY 4: NULL root returns NULL
    // ========================================
    auto check_null_safety = []() {
        assert(find_leaf_node(NULL, 0) == NULL && "NULL root must return NULL");
        assert(find_leaf_node(NULL, -100) == NULL && "NULL root must return NULL");
        assert(find_leaf_node(NULL, 100) == NULL && "NULL root must return NULL");
    };
    
    // ========================================
    // PROPERTY 5: Deterministic - same input gives same output
    // ========================================
    auto check_deterministic = [](Node* root, int key) {
        Node* result1 = find_leaf_node(root, key);
        Node* result2 = find_leaf_node(root, key);
        assert(result1 == result2 && "Function must be deterministic");
    };
    
    // ========================================
    // PROPERTY 6: Idempotent navigation - searching for any key 
    //             in result leaf returns same leaf
    // ========================================
    auto check_idempotent = [](Node* root, Node* result) {
        if (result == NULL || result->keys.empty()) return;
        
        for (int key : result->keys) {
            Node* re_search = find_leaf_node(root, key);
            assert(re_search == result && 
                   "Searching for keys in result must return same leaf");
        }
    };
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: NULL root
    check_null_safety();
    
    // Test 2: Root is leaf (single node tree)
    Node* root1 = new Node();
    root1->isLeaf = true;
    root1->keys = {5, 10, 15, 20};
    
    Node* result1 = find_leaf_node(root1, 10);
    assert(result1 == root1 && "Root leaf should return itself");
    check_is_leaf(result1);
    check_deterministic(root1, 10);
    check_idempotent(root1, result1);
    
    // Test with key not in tree but should still return root
    result1 = find_leaf_node(root1, 7);
    assert(result1 == root1 && "Should return root leaf for any key");
    check_is_leaf(result1);
    
    // Test 3: Empty leaf
    Node* root2 = new Node();
    root2->isLeaf = true;
    root2->keys = {};
    
    Node* result2 = find_leaf_node(root2, 10);
    assert(result2 == root2 && "Empty leaf should return itself");
    check_is_leaf(result2);
    
    // Test 4: Two-level tree - proper navigation
    Node* internal1 = new Node();
    internal1->isLeaf = false;
    internal1->keys = {10, 20};
    new (&internal1->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* leaf1 = new Node();
    leaf1->isLeaf = true;
    leaf1->keys = {3, 5, 8};
    
    Node* leaf2 = new Node();
    leaf2->isLeaf = true;
    leaf2->keys = {10, 12, 15};
    
    Node* leaf3 = new Node();
    leaf3->isLeaf = true;
    leaf3->keys = {20, 25, 30};
    
    internal1->ptr2TreeOrData.ptr2Tree.push_back(leaf1);
    internal1->ptr2TreeOrData.ptr2Tree.push_back(leaf2);
    internal1->ptr2TreeOrData.ptr2Tree.push_back(leaf3);
    
    // Test navigation to each leaf
    Node* r1 = find_leaf_node(internal1, 5);
    check_is_leaf(r1);
    check_reachable(internal1, r1);
    check_key_range_consistency(internal1, 5, r1);
    check_idempotent(internal1, r1);
    
    Node* r2 = find_leaf_node(internal1, 12);
    check_is_leaf(r2);
    check_reachable(internal1, r2);
    check_idempotent(internal1, r2);
    
    Node* r3 = find_leaf_node(internal1, 25);
    check_is_leaf(r3);
    check_reachable(internal1, r3);
    check_idempotent(internal1, r3);
    
    // Verify different leaves for different ranges
    assert(r1 != r2 && "Different key ranges should find different leaves");
    assert(r2 != r3 && "Different key ranges should find different leaves");
    
    check_deterministic(internal1, 5);
    check_deterministic(internal1, 12);
    check_deterministic(internal1, 25);
    
    // Test 5: Three-level tree
    Node* root3 = new Node();
    root3->isLeaf = false;
    root3->keys = {50};
    new (&root3->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* int_left = new Node();
    int_left->isLeaf = false;
    int_left->keys = {25};
    new (&int_left->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* int_right = new Node();
    int_right->isLeaf = false;
    int_right->keys = {75};
    new (&int_right->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* ll1 = new Node();
    ll1->isLeaf = true;
    ll1->keys = {10, 15, 20};
    
    Node* ll2 = new Node();
    ll2->isLeaf = true;
    ll2->keys = {25, 30, 40};
    
    Node* rl1 = new Node();
    rl1->isLeaf = true;
    rl1->keys = {50, 60, 70};
    
    Node* rl2 = new Node();
    rl2->isLeaf = true;
    rl2->keys = {75, 80, 90};
    
    int_left->ptr2TreeOrData.ptr2Tree.push_back(ll1);
    int_left->ptr2TreeOrData.ptr2Tree.push_back(ll2);
    int_right->ptr2TreeOrData.ptr2Tree.push_back(rl1);
    int_right->ptr2TreeOrData.ptr2Tree.push_back(rl2);
    
    root3->ptr2TreeOrData.ptr2Tree.push_back(int_left);
    root3->ptr2TreeOrData.ptr2Tree.push_back(int_right);
    
    // Test deep navigation
    Node* deep1 = find_leaf_node(root3, 15);
    check_is_leaf(deep1);
    check_reachable(root3, deep1);
    check_idempotent(root3, deep1);
    
    Node* deep2 = find_leaf_node(root3, 30);
    check_is_leaf(deep2);
    check_reachable(root3, deep2);
    
    Node* deep3 = find_leaf_node(root3, 60);
    check_is_leaf(deep3);
    check_reachable(root3, deep3);
    
    Node* deep4 = find_leaf_node(root3, 80);
    check_is_leaf(deep4);
    check_reachable(root3, deep4);
    
    // All should be different leaves
    assert(deep1 != deep2 && deep2 != deep3 && deep3 != deep4);
    
    // Test 6: Negative keys
    Node* root4 = new Node();
    root4->isLeaf = false;
    root4->keys = {0};
    new (&root4->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* neg_leaf = new Node();
    neg_leaf->isLeaf = true;
    neg_leaf->keys = {-50, -20, -10};
    
    Node* pos_leaf = new Node();
    pos_leaf->isLeaf = true;
    pos_leaf->keys = {0, 10, 20};
    
    root4->ptr2TreeOrData.ptr2Tree.push_back(neg_leaf);
    root4->ptr2TreeOrData.ptr2Tree.push_back(pos_leaf);
    
    Node* neg_result = find_leaf_node(root4, -30);
    check_is_leaf(neg_result);
    check_reachable(root4, neg_result);
    
    Node* pos_result = find_leaf_node(root4, 15);
    check_is_leaf(pos_result);
    check_reachable(root4, pos_result);
    
    // Test 7: Boundary values
    Node* root5 = new Node();
    root5->isLeaf = true;
    root5->keys = {INT_MIN, 0, INT_MAX};
    
    Node* r_min = find_leaf_node(root5, INT_MIN);
    Node* r_max = find_leaf_node(root5, INT_MAX);
    Node* r_zero = find_leaf_node(root5, 0);
    
    assert(r_min == root5 && r_max == root5 && r_zero == root5);
    check_is_leaf(r_min);
    
    // Test 8: Keys beyond tree range
    Node* result_beyond1 = find_leaf_node(internal1, -1000);
    check_is_leaf(result_beyond1);
    check_reachable(internal1, result_beyond1);
    
    Node* result_beyond2 = find_leaf_node(internal1, 1000);
    check_is_leaf(result_beyond2);
    check_reachable(internal1, result_beyond2);
    
    // Test 9: Stress test - wide tree
    Node* wide_root = new Node();
    wide_root->isLeaf = false;
    new (&wide_root->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    for (int i = 0; i < 10; i++) {
        wide_root->keys.push_back(i * 10);
    }
    
    for (int i = 0; i < 11; i++) {
        Node* wide_leaf = new Node();
        wide_leaf->isLeaf = true;
        wide_leaf->keys.push_back(i * 10);
        wide_root->ptr2TreeOrData.ptr2Tree.push_back(wide_leaf);
    }
    
    // Test navigation to each leaf
    for (int i = 0; i < 11; i++) {
        Node* wide_result = find_leaf_node(wide_root, i * 10);
        check_is_leaf(wide_result);
        check_reachable(wide_root, wide_result);
        check_deterministic(wide_root, i * 10);
    }
    
    std::cout << "All find_leaf_node strict tests passed!" << std::endl;
    return 0;
}
