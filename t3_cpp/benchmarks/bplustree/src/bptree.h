#pragma once
#include <vector>
#include <cstdio>

namespace bptree {

class Node {
public:
    bool isLeaf;
    std::vector<int> keys;
    Node* ptr2next;
    union ptr {
        std::vector<Node*> ptr2Tree;
        std::vector<FILE*> dataPtr;
        ptr();
        ~ptr();
    } ptr2TreeOrData;
    
    Node();
    ~Node();
};

// Standalone functions for benchmark
bool search_key(Node* root, int key);
Node* find_leaf_node(Node* root, int key);
int find_insert_position(const std::vector<int>& keys, int key);
void insert_into_sorted_leaf(Node* leaf, int key, FILE* filePtr);
Node* find_first_leaf(Node* root);
Node* find_parent(Node* root, Node* child);
int count_keys_in_level(Node* root, int level);

} // namespace bptree
