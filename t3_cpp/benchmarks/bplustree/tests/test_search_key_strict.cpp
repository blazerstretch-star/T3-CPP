#include "../src/bptree.h"
#include <cassert>
#include <iostream>
#include <set>
#include <vector>
#include <algorithm>
#include <climits>

using namespace bptree;

// Helper: Collect all keys in tree using BFS
std::set<int> collect_all_keys(Node* root) {
    std::set<int> all_keys;
    if (!root) return all_keys;
    
    std::vector<Node*> queue = {root};
    while (!queue.empty()) {
        Node* node = queue.back();
        queue.pop_back();
        
        for (int key : node->keys) {
            all_keys.insert(key);
        }
        
        if (!node->isLeaf) {
            for (Node* child : node->ptr2TreeOrData.ptr2Tree) {
                if (child) queue.push_back(child);
            }
        }
    }
    return all_keys;
}

int main() {
    // ========================================
    // PROPERTY 1: Completeness - all keys in tree must be found
    // ========================================
    auto check_completeness = [](Node* root) {
        std::set<int> all_keys = collect_all_keys(root);
        for (int key : all_keys) {
            assert(search_key(root, key) == true && 
                   "All keys present in tree must be found");
        }
    };
    
    // ========================================
    // PROPERTY 2: Soundness - keys not in tree must not be found
    // ========================================
    auto check_soundness = [](Node* root, const std::vector<int>& absent_keys) {
        for (int key : absent_keys) {
            assert(search_key(root, key) == false && 
                   "Keys not in tree must return false");
        }
    };
    
    // ========================================
    // PROPERTY 3: NULL safety - NULL root returns false
    // ========================================
    auto check_null_safety = []() {
        assert(search_key(NULL, 0) == false && "NULL root must return false");
        assert(search_key(NULL, -100) == false && "NULL root must return false");
        assert(search_key(NULL, 100) == false && "NULL root must return false");
    };
    
    // ========================================
    // PROPERTY 4: Deterministic - same input gives same output
    // ========================================
    auto check_deterministic = [](Node* root, int key) {
        bool result1 = search_key(root, key);
        bool result2 = search_key(root, key);
        assert(result1 == result2 && "Search must be deterministic");
    };
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: NULL root
    check_null_safety();
    
    // Test 2: Single leaf node - basic functionality
    Node* root1 = new Node();
    root1->isLeaf = true;
    root1->keys = {5, 10, 15, 20, 25};
    
    check_completeness(root1);
    check_soundness(root1, {1, 3, 7, 12, 18, 30, 100});
    check_deterministic(root1, 10);
    check_deterministic(root1, 7);
    
    // Test 3: Empty leaf node
    Node* root2 = new Node();
    root2->isLeaf = true;
    root2->keys = {};
    
    assert(search_key(root2, 10) == false && "Empty tree returns false");
    check_completeness(root2);
    
    // Test 4: Two-level tree
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
    
    check_completeness(internal1);
    check_soundness(internal1, {1, 2, 4, 6, 7, 9, 11, 13, 16, 22, 35, 100});
    check_deterministic(internal1, 10);
    check_deterministic(internal1, 7);
    
    // Test 5: Three-level tree (stress test)
    Node* root3 = new Node();
    root3->isLeaf = false;
    root3->keys = {50};
    new (&root3->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* int_left = new Node();
    int_left->isLeaf = false;
    int_left->keys = {20, 40};
    new (&int_left->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* int_right = new Node();
    int_right->isLeaf = false;
    int_right->keys = {70, 90};
    new (&int_right->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    // Left subtree leaves
    Node* ll1 = new Node();
    ll1->isLeaf = true;
    ll1->keys = {5, 10, 15};
    
    Node* ll2 = new Node();
    ll2->isLeaf = true;
    ll2->keys = {20, 25, 30, 35};
    
    Node* ll3 = new Node();
    ll3->isLeaf = true;
    ll3->keys = {40, 45};
    
    int_left->ptr2TreeOrData.ptr2Tree.push_back(ll1);
    int_left->ptr2TreeOrData.ptr2Tree.push_back(ll2);
    int_left->ptr2TreeOrData.ptr2Tree.push_back(ll3);
    
    // Right subtree leaves
    Node* rl1 = new Node();
    rl1->isLeaf = true;
    rl1->keys = {50, 55, 60, 65};
    
    Node* rl2 = new Node();
    rl2->isLeaf = true;
    rl2->keys = {70, 75, 80, 85};
    
    Node* rl3 = new Node();
    rl3->isLeaf = true;
    rl3->keys = {90, 95, 100};
    
    int_right->ptr2TreeOrData.ptr2Tree.push_back(rl1);
    int_right->ptr2TreeOrData.ptr2Tree.push_back(rl2);
    int_right->ptr2TreeOrData.ptr2Tree.push_back(rl3);
    
    root3->ptr2TreeOrData.ptr2Tree.push_back(int_left);
    root3->ptr2TreeOrData.ptr2Tree.push_back(int_right);
    
    check_completeness(root3);
    check_soundness(root3, {1, 3, 12, 18, 33, 48, 58, 88, 105, 200});
    
    // Test 6: Negative keys
    Node* root4 = new Node();
    root4->isLeaf = true;
    root4->keys = {-50, -20, -10, 0, 10, 20, 50};
    
    check_completeness(root4);
    check_soundness(root4, {-100, -30, -5, 5, 15, 100});
    
    // Test 7: Duplicate keys in different leaves
    Node* internal2 = new Node();
    internal2->isLeaf = false;
    internal2->keys = {10};
    new (&internal2->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* dup_leaf1 = new Node();
    dup_leaf1->isLeaf = true;
    dup_leaf1->keys = {5, 10};
    
    Node* dup_leaf2 = new Node();
    dup_leaf2->isLeaf = true;
    dup_leaf2->keys = {10, 15};
    
    internal2->ptr2TreeOrData.ptr2Tree.push_back(dup_leaf1);
    internal2->ptr2TreeOrData.ptr2Tree.push_back(dup_leaf2);
    
    // Should find at least one occurrence
    assert(search_key(internal2, 10) == true && "Duplicate keys must be found");
    check_completeness(internal2);
    
    // Test 8: Large tree (stress test)
    Node* large_root = new Node();
    large_root->isLeaf = true;
    for (int i = 0; i < 100; i++) {
        large_root->keys.push_back(i * 10);
    }
    
    check_completeness(large_root);
    // Test some absent keys
    std::vector<int> absent;
    for (int i = 0; i < 100; i++) {
        absent.push_back(i * 10 + 5); // Between existing keys
    }
    check_soundness(large_root, absent);
    
    // Test 9: Boundary values
    Node* root5 = new Node();
    root5->isLeaf = true;
    root5->keys = {INT_MIN, -1, 0, 1, INT_MAX};
    
    check_completeness(root5);
    std::vector<int> boundary_absent = {INT_MIN + 1, -2, 2, INT_MAX - 1};
    check_soundness(root5, boundary_absent);
    
    std::cout << "All search_key strict tests passed!" << std::endl;
    return 0;
}
