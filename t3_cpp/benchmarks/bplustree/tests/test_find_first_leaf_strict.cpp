#include "../src/bptree.h"
#include <cassert>
#include <iostream>
#include <vector>
#include <queue>

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
    // PROPERTY 3: Result is leftmost leaf - follows leftmost path
    //             (DFS traversal taking first child at each level)
    // ========================================
    auto check_is_leftmost = [](Node* root, Node* result) {
        if (result == NULL || root == NULL) return;
        
        // Follow leftmost path (first non-null child at each level)
        Node* cursor = root;
        while (!cursor->isLeaf) {
            if (cursor->ptr2TreeOrData.ptr2Tree.empty()) break;
            
            // Find first non-null child
            Node* first_child = NULL;
            for (Node* child : cursor->ptr2TreeOrData.ptr2Tree) {
                if (child != NULL) {
                    first_child = child;
                    break;
                }
            }
            
            if (first_child == NULL) break;
            cursor = first_child;
        }
        
        assert(cursor == result && 
               "Result must be the leftmost leaf (first non-null child path)");
    };
    
    // ========================================
    // PROPERTY 4: NULL root returns NULL
    // ========================================
    auto check_null_safety = []() {
        assert(find_first_leaf(NULL) == NULL && "NULL root must return NULL");
    };
    
    // ========================================
    // PROPERTY 5: Deterministic - same input gives same output
    // ========================================
    auto check_deterministic = [](Node* root) {
        Node* result1 = find_first_leaf(root);
        Node* result2 = find_first_leaf(root);
        assert(result1 == result2 && "Function must be deterministic");
    };
    
    // ========================================
    // PROPERTY 6: If root is leaf, returns root
    // ========================================
    auto check_root_is_leaf = [](Node* root) {
        if (root && root->isLeaf) {
            Node* result = find_first_leaf(root);
            assert(result == root && "If root is leaf, must return root");
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
    root1->keys = {5, 10, 15};
    
    Node* result1 = find_first_leaf(root1);
    assert(result1 == root1 && "Root leaf should return itself");
    check_is_leaf(result1);
    check_root_is_leaf(root1);
    check_deterministic(root1);
    
    // Test 3: Empty leaf (root)
    Node* root2 = new Node();
    root2->isLeaf = true;
    root2->keys = {};
    
    Node* result2 = find_first_leaf(root2);
    assert(result2 == root2 && "Empty root leaf should return itself");
    check_is_leaf(result2);
    check_root_is_leaf(root2);
    
    // Test 4: Two-level tree - leftmost leaf
    Node* internal1 = new Node();
    internal1->isLeaf = false;
    internal1->keys = {20, 40};
    new (&internal1->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* left1 = new Node();
    left1->isLeaf = true;
    left1->keys = {5, 10, 15};
    
    Node* middle1 = new Node();
    middle1->isLeaf = true;
    middle1->keys = {20, 25, 30};
    
    Node* right1 = new Node();
    right1->isLeaf = true;
    right1->keys = {40, 50, 60};
    
    internal1->ptr2TreeOrData.ptr2Tree.push_back(left1);
    internal1->ptr2TreeOrData.ptr2Tree.push_back(middle1);
    internal1->ptr2TreeOrData.ptr2Tree.push_back(right1);
    
    Node* result3 = find_first_leaf(internal1);
    assert(result3 == left1 && "Should return leftmost leaf");
    check_is_leaf(result3);
    check_reachable(internal1, result3);
    check_is_leftmost(internal1, result3);
    check_deterministic(internal1);
    
    // Test 5: Three-level tree - deep leftmost
    Node* root3 = new Node();
    root3->isLeaf = false;
    root3->keys = {50};
    new (&root3->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* int_left = new Node();
    int_left->isLeaf = false;
    int_left->keys = {20, 35};
    new (&int_left->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* int_right = new Node();
    int_right->isLeaf = false;
    int_right->keys = {70};
    new (&int_right->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* ll1 = new Node();
    ll1->isLeaf = true;
    ll1->keys = {5, 10, 15}; // This should be the first leaf
    
    Node* ll2 = new Node();
    ll2->isLeaf = true;
    ll2->keys = {20, 25, 30};
    
    Node* ll3 = new Node();
    ll3->isLeaf = true;
    ll3->keys = {35, 40, 45};
    
    Node* rl1 = new Node();
    rl1->isLeaf = true;
    rl1->keys = {50, 60};
    
    Node* rl2 = new Node();
    rl2->isLeaf = true;
    rl2->keys = {70, 80, 90};
    
    int_left->ptr2TreeOrData.ptr2Tree.push_back(ll1);
    int_left->ptr2TreeOrData.ptr2Tree.push_back(ll2);
    int_left->ptr2TreeOrData.ptr2Tree.push_back(ll3);
    
    int_right->ptr2TreeOrData.ptr2Tree.push_back(rl1);
    int_right->ptr2TreeOrData.ptr2Tree.push_back(rl2);
    
    root3->ptr2TreeOrData.ptr2Tree.push_back(int_left);
    root3->ptr2TreeOrData.ptr2Tree.push_back(int_right);
    
    Node* result4 = find_first_leaf(root3);
    assert(result4 == ll1 && "Should return deepest leftmost leaf");
    check_is_leaf(result4);
    check_reachable(root3, result4);
    check_is_leftmost(root3, result4);
    check_deterministic(root3);
    
    // Test 6: Four-level tree (stress depth)
    Node* root4 = new Node();
    root4->isLeaf = false;
    root4->keys = {100};
    new (&root4->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* l2_1 = new Node();
    l2_1->isLeaf = false;
    l2_1->keys = {50};
    new (&l2_1->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* l2_2 = new Node();
    l2_2->isLeaf = false;
    l2_2->keys = {150};
    new (&l2_2->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* l3_1 = new Node();
    l3_1->isLeaf = false;
    l3_1->keys = {25};
    new (&l3_1->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* l3_2 = new Node();
    l3_2->isLeaf = false;
    l3_2->keys = {75};
    new (&l3_2->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* leftmost_leaf = new Node();
    leftmost_leaf->isLeaf = true;
    leftmost_leaf->keys = {1, 2, 3};
    
    Node* other_leaf = new Node();
    other_leaf->isLeaf = true;
    other_leaf->keys = {26, 27};
    
    Node* another_leaf = new Node();
    another_leaf->isLeaf = true;
    another_leaf->keys = {75, 80};
    
    Node* far_leaf = new Node();
    far_leaf->isLeaf = true;
    far_leaf->keys = {150, 200};
    
    l3_1->ptr2TreeOrData.ptr2Tree.push_back(leftmost_leaf);
    l3_1->ptr2TreeOrData.ptr2Tree.push_back(other_leaf);
    
    l3_2->ptr2TreeOrData.ptr2Tree.push_back(another_leaf);
    
    l2_1->ptr2TreeOrData.ptr2Tree.push_back(l3_1);
    l2_1->ptr2TreeOrData.ptr2Tree.push_back(l3_2);
    
    l2_2->ptr2TreeOrData.ptr2Tree.push_back(far_leaf);
    
    root4->ptr2TreeOrData.ptr2Tree.push_back(l2_1);
    root4->ptr2TreeOrData.ptr2Tree.push_back(l2_2);
    
    Node* result5 = find_first_leaf(root4);
    assert(result5 == leftmost_leaf && "Should find deepest leftmost leaf");
    check_is_leaf(result5);
    check_reachable(root4, result5);
    check_is_leftmost(root4, result5);
    check_deterministic(root4);
    
    // Test 7: Wide tree (many children at root)
    Node* wide_root = new Node();
    wide_root->isLeaf = false;
    new (&wide_root->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    for (int i = 0; i < 10; i++) {
        wide_root->keys.push_back(i * 10);
    }
    
    Node* first_wide_leaf = NULL;
    for (int i = 0; i < 11; i++) {
        Node* wide_leaf = new Node();
        wide_leaf->isLeaf = true;
        wide_leaf->keys.push_back(i * 10);
        wide_root->ptr2TreeOrData.ptr2Tree.push_back(wide_leaf);
        
        if (i == 0) first_wide_leaf = wide_leaf;
    }
    
    Node* result6 = find_first_leaf(wide_root);
    assert(result6 == first_wide_leaf && "Should return first child in wide tree");
    check_is_leaf(result6);
    check_reachable(wide_root, result6);
    check_is_leftmost(wide_root, result6);
    
    // Test 8: Tree with empty leaves
    Node* root5 = new Node();
    root5->isLeaf = false;
    root5->keys = {10};
    new (&root5->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* empty_leaf1 = new Node();
    empty_leaf1->isLeaf = true;
    empty_leaf1->keys = {}; // Empty
    
    Node* non_empty_leaf = new Node();
    non_empty_leaf->isLeaf = true;
    non_empty_leaf->keys = {10, 20};
    
    root5->ptr2TreeOrData.ptr2Tree.push_back(empty_leaf1);
    root5->ptr2TreeOrData.ptr2Tree.push_back(non_empty_leaf);
    
    Node* result7 = find_first_leaf(root5);
    assert(result7 == empty_leaf1 && "Should return first leaf even if empty");
    check_is_leaf(result7);
    check_reachable(root5, result7);
    
    // Test 9: Unbalanced tree (left heavy)
    Node* unbal_root = new Node();
    unbal_root->isLeaf = false;
    unbal_root->keys = {100};
    new (&unbal_root->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* deep_internal = new Node();
    deep_internal->isLeaf = false;
    deep_internal->keys = {50};
    new (&deep_internal->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* deep_leaf = new Node();
    deep_leaf->isLeaf = true;
    deep_leaf->keys = {1};
    
    Node* shallow_leaf = new Node();
    shallow_leaf->isLeaf = true;
    shallow_leaf->keys = {100};
    
    deep_internal->ptr2TreeOrData.ptr2Tree.push_back(deep_leaf);
    deep_internal->ptr2TreeOrData.ptr2Tree.push_back(shallow_leaf);
    
    unbal_root->ptr2TreeOrData.ptr2Tree.push_back(deep_internal);
    
    Node* result8 = find_first_leaf(unbal_root);
    assert(result8 == deep_leaf && "Should find leftmost even in unbalanced tree");
    check_is_leaf(result8);
    check_reachable(unbal_root, result8);
    
    std::cout << "All find_first_leaf strict tests passed!" << std::endl;
    return 0;
}
