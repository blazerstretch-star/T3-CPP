#include "../src/bptree.h"
#include <cassert>
#include <iostream>

using namespace bptree;

int main() {
    // Test 1: NULL root
    assert(find_leaf_node(NULL, 10) == NULL);
    
    // Test 2: Root is leaf
    Node* root = new Node();
    root->isLeaf = true;
    root->keys = {5, 10, 15};
    assert(find_leaf_node(root, 10) == root);
    assert(find_leaf_node(root, 5) == root);
    
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
    right->keys = {10, 15};
    
    internal->ptr2TreeOrData.ptr2Tree.push_back(left);
    internal->ptr2TreeOrData.ptr2Tree.push_back(right);
    
    assert(find_leaf_node(internal, 5) == left);
    assert(find_leaf_node(internal, 8) == left);
    assert(find_leaf_node(internal, 10) == right);
    assert(find_leaf_node(internal, 15) == right);
    assert(find_leaf_node(internal, 20) == right);
    
    std::cout << "All find_leaf_node tests passed!" << std::endl;
    return 0;
}
