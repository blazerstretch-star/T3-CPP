#include "../src/bptree.h"
#include <cassert>
#include <iostream>

using namespace bptree;

int main() {
    // Test 1: NULL inputs
    assert(find_parent(NULL, NULL) == NULL);
    
    // Test 2: Root is leaf (no parent possible)
    Node* leaf = new Node();
    leaf->isLeaf = true;
    assert(find_parent(leaf, leaf) == NULL);
    
    // Test 3: Two-level tree - direct children
    Node* root = new Node();
    root->isLeaf = false;
    root->keys = {10};
    new (&root->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* left = new Node();
    left->isLeaf = false;
    left->keys = {5};
    new (&left->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* right = new Node();
    right->isLeaf = false;
    right->keys = {15};
    new (&right->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* leaf1 = new Node();
    leaf1->isLeaf = true;
    
    Node* leaf2 = new Node();
    leaf2->isLeaf = true;
    
    left->ptr2TreeOrData.ptr2Tree.push_back(leaf1);
    right->ptr2TreeOrData.ptr2Tree.push_back(leaf2);
    
    root->ptr2TreeOrData.ptr2Tree.push_back(left);
    root->ptr2TreeOrData.ptr2Tree.push_back(right);
    
    Node* parent = find_parent(root, left);
    assert(parent != NULL);
    assert(parent == root);
    
    parent = find_parent(root, right);
    assert(parent != NULL);
    assert(parent == root);
    
    // Test 4: Three-level tree - recursive search
    Node* root3 = new Node();
    root3->isLeaf = false;
    root3->keys = {20};
    new (&root3->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* internal = new Node();
    internal->isLeaf = false;
    internal->keys = {25};
    new (&internal->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* child = new Node();
    child->isLeaf = false;
    child->keys = {30};
    new (&child->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
    
    Node* leaf3 = new Node();
    leaf3->isLeaf = true;
    
    child->ptr2TreeOrData.ptr2Tree.push_back(leaf3);
    internal->ptr2TreeOrData.ptr2Tree.push_back(child);
    internal->ptr2TreeOrData.ptr2Tree.push_back(right);
    root3->ptr2TreeOrData.ptr2Tree.push_back(internal);
    root3->ptr2TreeOrData.ptr2Tree.push_back(left);
    
    parent = find_parent(root3, internal);
    assert(parent != NULL);
    assert(parent == root3);
    
    parent = find_parent(root3, child);
    assert(parent != NULL);
    assert(parent == internal);
    
    std::cout << "All find_parent tests passed!" << std::endl;
    return 0;
}
