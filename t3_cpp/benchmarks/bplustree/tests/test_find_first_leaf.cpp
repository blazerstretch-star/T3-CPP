#include "../src/bptree.h"
#include <cassert>
#include <iostream>

using namespace bptree;

int main() {
    // Test 1: NULL root
    assert(find_first_leaf(NULL) == NULL);
    
    // Test 2: Root is leaf
    Node* root = new Node();
    root->isLeaf = true;
    root->keys = {5, 10};
    assert(find_first_leaf(root) == root);
    
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
    
    assert(find_first_leaf(internal) == left);
    
    // Test 4: Three-level tree
    Node* root3 = new Node();
    root3->isLeaf = false;
    root3->keys = {20};
    new (&root3->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* internal2 = new Node();
    internal2->isLeaf = false;
    internal2->keys = {30};
    new (&internal2->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* leftmost = new Node();
    leftmost->isLeaf = true;
    leftmost->keys = {1, 2};
    
    Node* middle = new Node();
    middle->isLeaf = true;
    middle->keys = {25, 28};
    
    internal2->ptr2TreeOrData.ptr2Tree.push_back(leftmost);
    internal2->ptr2TreeOrData.ptr2Tree.push_back(middle);
    
    root3->ptr2TreeOrData.ptr2Tree.push_back(internal2);
    root3->ptr2TreeOrData.ptr2Tree.push_back(right);
    
    assert(find_first_leaf(root3) == leftmost);
    
    std::cout << "All find_first_leaf tests passed!" << std::endl;
    return 0;
}
