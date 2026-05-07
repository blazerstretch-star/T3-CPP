#include "../src/bptree.h"
#include <cassert>
#include <iostream>
#include <vector>

using namespace bptree;

int main() {
    // ========================================
    // PROPERTY 1: If result is not NULL, *result must contain child
    // ========================================
    auto check_parent_contains_child = [](Node* parent, Node* child) {
        if (parent == NULL) return;
        
        assert(!parent->isLeaf && "Parent must not be a leaf");
        
        bool found = false;
        for (Node* c : parent->ptr2TreeOrData.ptr2Tree) {
            if (c == child) {
                found = true;
                break;
            }
        }
        assert(found && "Parent must contain child in its ptr2Tree");
    };
    
    // ========================================
    // PROPERTY 2: Parent must not be a leaf
    // ========================================
    auto check_parent_not_leaf = [](Node* parent) {
        if (parent != NULL) {
            assert(parent->isLeaf == false && 
                   "Parent node must be internal (not leaf)");
        }
    };
    
    // ========================================
    // PROPERTY 3: NULL inputs return NULL
    // ========================================
    auto check_null_safety = []() {
        assert(find_parent(NULL, NULL) == NULL && "NULL inputs must return NULL");
        
        Node* dummy = new Node();
        assert(find_parent(NULL, dummy) == NULL && "NULL root must return NULL");
        assert(find_parent(dummy, NULL) == NULL && "NULL child must return NULL");
    };
    
    // ========================================
    // PROPERTY 4: Leaf root returns NULL (no parent possible)
    // ========================================
    auto check_leaf_root = []() {
        Node* leaf = new Node();
        leaf->isLeaf = true;
        
        assert(find_parent(leaf, leaf) == NULL && 
               "Leaf root has no parent");
    };
    
    // ========================================
    // PROPERTY 5: Deterministic - same input gives same output
    // ========================================
    auto check_deterministic = [](Node* root, Node* child) {
        Node* result1 = find_parent(root, child);
        Node* result2 = find_parent(root, child);
        
        if (result1 == NULL && result2 == NULL) return;
        if (result1 != NULL && result2 != NULL) {
            assert(result1 == result2 && "Function must be deterministic");
        } else {
            assert(false && "Inconsistent NULL/non-NULL results");
        }
    };
    
    // ========================================
    // PROPERTY 6: Child must be reachable from root
    // ========================================
    auto check_child_reachable = [](Node* root, Node* child, Node* result) {
        if (result == NULL) return; // Child not found, might not be in tree
        
        // Verify child is actually in the tree
        bool found = false;
        std::vector<Node*> queue = {root};
        while (!queue.empty() && !found) {
            Node* node = queue.back();
            queue.pop_back();
            
            if (node == child) {
                found = true;
                break;
            }
            
            if (!node->isLeaf) {
                for (Node* c : node->ptr2TreeOrData.ptr2Tree) {
                    if (c) queue.push_back(c);
                }
            }
        }
        assert(found && "Child must be reachable from root if parent found");
    };
    
    // ========================================
    // PROPERTY 7: Parent must be reachable from root
    // ========================================
    auto check_parent_reachable = [](Node* root, Node* parent) {
        if (parent == NULL) return;
        if (parent == root) return; // Root is trivially reachable
        
        bool found = false;
        std::vector<Node*> queue = {root};
        while (!queue.empty() && !found) {
            Node* node = queue.back();
            queue.pop_back();
            
            if (node == parent) {
                found = true;
                break;
            }
            
            if (!node->isLeaf) {
                for (Node* c : node->ptr2TreeOrData.ptr2Tree) {
                    if (c) queue.push_back(c);
                }
            }
        }
        assert(found && "Parent must be reachable from root");
    };
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: NULL safety
    check_null_safety();
    
    // Test 2: Leaf root (no parent possible)
    check_leaf_root();
    
    // Test 3: Two-level tree - direct children
    Node* root1 = new Node();
    root1->isLeaf = false;
    root1->keys = {20};
    new (&root1->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* left1 = new Node();
    left1->isLeaf = false;
    left1->keys = {10};
    new (&left1->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* right1 = new Node();
    right1->isLeaf = false;
    right1->keys = {30};
    new (&right1->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* leaf1 = new Node();
    leaf1->isLeaf = true;
    
    Node* leaf2 = new Node();
    leaf2->isLeaf = true;
    
    left1->ptr2TreeOrData.ptr2Tree.push_back(leaf1);
    right1->ptr2TreeOrData.ptr2Tree.push_back(leaf2);
    
    root1->ptr2TreeOrData.ptr2Tree.push_back(left1);
    root1->ptr2TreeOrData.ptr2Tree.push_back(right1);
    
    // Find parent of left1
    Node* parent_left = find_parent(root1, left1);
    assert(parent_left != NULL && "Parent should be found");
    assert(parent_left == root1 && "Parent should be root");
    check_parent_contains_child(parent_left, left1);
    check_parent_not_leaf(parent_left);
    check_parent_reachable(root1, parent_left);
    check_deterministic(root1, left1);
    
    // Find parent of right1
    Node* parent_right = find_parent(root1, right1);
    assert(parent_right != NULL && "Parent should be found");
    assert(parent_right == root1 && "Parent should be root");
    check_parent_contains_child(parent_right, right1);
    check_parent_not_leaf(parent_right);
    check_deterministic(root1, right1);
    
    // Test 4: Three-level tree - recursive search
    Node* root2 = new Node();
    root2->isLeaf = false;
    root2->keys = {50};
    new (&root2->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* int_left = new Node();
    int_left->isLeaf = false;
    int_left->keys = {25};
    new (&int_left->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* int_right = new Node();
    int_right->isLeaf = false;
    int_right->keys = {75};
    new (&int_right->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* deep_child1 = new Node();
    deep_child1->isLeaf = false;
    deep_child1->keys = {10};
    new (&deep_child1->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* deep_child2 = new Node();
    deep_child2->isLeaf = false;
    deep_child2->keys = {40};
    new (&deep_child2->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* ll1 = new Node();
    ll1->isLeaf = true;
    
    Node* ll2 = new Node();
    ll2->isLeaf = true;
    
    deep_child1->ptr2TreeOrData.ptr2Tree.push_back(ll1);
    deep_child2->ptr2TreeOrData.ptr2Tree.push_back(ll2);
    
    int_left->ptr2TreeOrData.ptr2Tree.push_back(deep_child1);
    int_left->ptr2TreeOrData.ptr2Tree.push_back(deep_child2);
    
    root2->ptr2TreeOrData.ptr2Tree.push_back(int_left);
    root2->ptr2TreeOrData.ptr2Tree.push_back(int_right);
    
    // Find parent of int_left (should be root2)
    Node* p1 = find_parent(root2, int_left);
    assert(p1 != NULL && p1 == root2);
    check_parent_contains_child(p1, int_left);
    check_parent_not_leaf(p1);
    check_parent_reachable(root2, p1);
    check_deterministic(root2, int_left);
    
    // Find parent of deep_child1 (should be int_left)
    Node* p2 = find_parent(root2, deep_child1);
    assert(p2 != NULL && p2 == int_left);
    check_parent_contains_child(p2, deep_child1);
    check_parent_not_leaf(p2);
    check_parent_reachable(root2, p2);
    check_deterministic(root2, deep_child1);
    
    // Find parent of deep_child2 (should be int_left)
    Node* p3 = find_parent(root2, deep_child2);
    assert(p3 != NULL && p3 == int_left);
    check_parent_contains_child(p3, deep_child2);
    check_deterministic(root2, deep_child2);
    
    // Test 5: Four-level tree (deep recursion)
    Node* root3 = new Node();
    root3->isLeaf = false;
    root3->keys = {100};
    new (&root3->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* l2 = new Node();
    l2->isLeaf = false;
    l2->keys = {50};
    new (&l2->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* l3 = new Node();
    l3->isLeaf = false;
    l3->keys = {25};
    new (&l3->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* l4 = new Node();
    l4->isLeaf = false;
    l4->keys = {10};
    new (&l4->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* deep_leaf = new Node();
    deep_leaf->isLeaf = true;
    
    l4->ptr2TreeOrData.ptr2Tree.push_back(deep_leaf);
    l3->ptr2TreeOrData.ptr2Tree.push_back(l4);
    l2->ptr2TreeOrData.ptr2Tree.push_back(l3);
    root3->ptr2TreeOrData.ptr2Tree.push_back(l2);
    
    // Find parent at each level
    Node* p_l2 = find_parent(root3, l2);
    assert(p_l2 != NULL && p_l2 == root3);
    check_parent_contains_child(p_l2, l2);
    check_deterministic(root3, l2);
    
    Node* p_l3 = find_parent(root3, l3);
    assert(p_l3 != NULL && p_l3 == l2);
    check_parent_contains_child(p_l3, l3);
    check_deterministic(root3, l3);
    
    Node* p_l4 = find_parent(root3, l4);
    assert(p_l4 != NULL && p_l4 == l3);
    check_parent_contains_child(p_l4, l4);
    check_deterministic(root3, l4);
    
    // Test 6: Wide tree (many siblings)
    Node* wide_root = new Node();
    wide_root->isLeaf = false;
    new (&wide_root->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    for (int i = 0; i < 10; i++) {
        wide_root->keys.push_back(i * 10);
    }
    
    std::vector<Node*> children;
    for (int i = 0; i < 11; i++) {
        Node* child = new Node();
        child->isLeaf = false;
        child->keys.push_back(i * 10);
        new (&child->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
        
        Node* grandchild = new Node();
        grandchild->isLeaf = true;
        child->ptr2TreeOrData.ptr2Tree.push_back(grandchild);
        
        wide_root->ptr2TreeOrData.ptr2Tree.push_back(child);
        children.push_back(child);
    }
    
    // Test finding parent for each child
    for (Node* child : children) {
        Node* p = find_parent(wide_root, child);
        assert(p != NULL && p == wide_root);
        check_parent_contains_child(p, child);
        check_deterministic(wide_root, child);
    }
    
    // Test 7: Node not in tree
    Node* orphan = new Node();
    orphan->isLeaf = false;
    new (&orphan->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* p_orphan = find_parent(root1, orphan);
    // Should return NULL since orphan is not in tree
    // (implementation may vary, but should not crash)
    
    // Test 8: Root has no parent
    Node* p_root = find_parent(root1, root1);
    // Root cannot be its own parent (should return NULL)
    
    // Test 9: Multiple paths to same node (shouldn't happen in tree, but test robustness)
    Node* root4 = new Node();
    root4->isLeaf = false;
    root4->keys = {50};
    new (&root4->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* shared_child = new Node();
    shared_child->isLeaf = false;
    shared_child->keys = {25};
    new (&shared_child->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* sc_leaf = new Node();
    sc_leaf->isLeaf = true;
    
    shared_child->ptr2TreeOrData.ptr2Tree.push_back(sc_leaf);
    root4->ptr2TreeOrData.ptr2Tree.push_back(shared_child);
    
    Node* p_shared = find_parent(root4, shared_child);
    assert(p_shared != NULL && p_shared == root4);
    check_parent_contains_child(p_shared, shared_child);
    
    // Test 10: Empty ptr2Tree (edge case)
    Node* empty_internal = new Node();
    empty_internal->isLeaf = false;
    new (&empty_internal->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    // Empty children vector
    
    Node* some_node = new Node();
    some_node->isLeaf = false;
    
    Node* p_empty = find_parent(empty_internal, some_node);
    // Should return NULL (no children to search)
    
    std::cout << "All find_parent strict tests passed!" << std::endl;
    return 0;
}
