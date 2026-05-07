#include "../src/bptree.h"
#include <cassert>
#include <iostream>

using namespace bptree;

int main() {
    // Test 1: NULL root
    assert(count_keys_in_level(NULL, 0) == 0);
    
    // Test 2: Single level (root only)
    Node* root = new Node();
    root->isLeaf = true;
    root->keys = {5, 10, 15};
    
    assert(count_keys_in_level(root, 0) == 3);
    assert(count_keys_in_level(root, 1) == 0);
    assert(count_keys_in_level(root, 2) == 0);
    
    // Test 3: Two-level tree
    Node* internal = new Node();
    internal->isLeaf = false;
    internal->keys = {10};
    new (&internal->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* left = new Node();
    left->isLeaf = true;
    left->keys = {5, 8};
    
    Node* right = new Node();
    right->isLeaf = true;
    right->keys = {10, 15, 20};
    
    internal->ptr2TreeOrData.ptr2Tree.push_back(left);
    internal->ptr2TreeOrData.ptr2Tree.push_back(right);
    
    assert(count_keys_in_level(internal, 0) == 1);  // Root: 1 key
    assert(count_keys_in_level(internal, 1) == 5);  // Leaves: 2+3=5 keys
    assert(count_keys_in_level(internal, 2) == 0);  // No level 2
    
    // Test 4: Three-level tree
    Node* root3 = new Node();
    root3->isLeaf = false;
    root3->keys = {20, 40};  // 2 keys at root
    new (&root3->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* int1 = new Node();
    int1->isLeaf = false;
    int1->keys = {10};  // 1 key
    new (&int1->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* int2 = new Node();
    int2->isLeaf = false;
    int2->keys = {30};  // 1 key
    new (&int2->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* int3 = new Node();
    int3->isLeaf = false;
    int3->keys = {50, 60};  // 2 keys
    new (&int3->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* leaf1 = new Node();
    leaf1->isLeaf = true;
    leaf1->keys = {5};  // 1 key
    
    Node* leaf2 = new Node();
    leaf2->isLeaf = true;
    leaf2->keys = {15};  // 1 key
    
    Node* leaf3 = new Node();
    leaf3->isLeaf = true;
    leaf3->keys = {25};  // 1 key
    
    Node* leaf4 = new Node();
    leaf4->isLeaf = true;
    leaf4->keys = {35};  // 1 key
    
    Node* leaf5 = new Node();
    leaf5->isLeaf = true;
    leaf5->keys = {45};  // 1 key
    
    Node* leaf6 = new Node();
    leaf6->isLeaf = true;
    leaf6->keys = {55, 65};  // 2 keys
    
    int1->ptr2TreeOrData.ptr2Tree.push_back(leaf1);
    int1->ptr2TreeOrData.ptr2Tree.push_back(leaf2);
    
    int2->ptr2TreeOrData.ptr2Tree.push_back(leaf3);
    int2->ptr2TreeOrData.ptr2Tree.push_back(leaf4);
    
    int3->ptr2TreeOrData.ptr2Tree.push_back(leaf5);
    int3->ptr2TreeOrData.ptr2Tree.push_back(leaf6);
    
    root3->ptr2TreeOrData.ptr2Tree.push_back(int1);
    root3->ptr2TreeOrData.ptr2Tree.push_back(int2);
    root3->ptr2TreeOrData.ptr2Tree.push_back(int3);
    
    assert(count_keys_in_level(root3, 0) == 2);   // Root: 2 keys
    assert(count_keys_in_level(root3, 1) == 4);   // Internal: 1+1+2=4 keys
    assert(count_keys_in_level(root3, 2) == 7);   // Leaves: 1+1+1+1+1+2=7 keys
    assert(count_keys_in_level(root3, 3) == 0);   // No level 3
    
    std::cout << "All count_keys_in_level tests passed!" << std::endl;
    return 0;
}
