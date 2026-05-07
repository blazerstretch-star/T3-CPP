#include "../src/bptree.h"
#include <cassert>
#include <iostream>

using namespace bptree;

int main() {
    // Test 1: NULL root
    assert(search_key(NULL, 10) == false);
    
    // Test 2: Single leaf node
    Node* root = new Node();
    root->isLeaf = true;
    root->keys = {5, 10, 15};
    
    assert(search_key(root, 10) == true);
    assert(search_key(root, 5) == true);
    assert(search_key(root, 15) == true);
    assert(search_key(root, 20) == false);
    assert(search_key(root, 1) == false);
    
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
    
    assert(search_key(internal, 5) == true);
    assert(search_key(internal, 8) == true);
    assert(search_key(internal, 10) == true);
    assert(search_key(internal, 15) == true);
    assert(search_key(internal, 3) == false);
    assert(search_key(internal, 20) == false);
    
    std::cout << "All search_key tests passed!" << std::endl;
    return 0;
}
