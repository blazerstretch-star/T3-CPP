#include "../src/bptree.h"
#include <cassert>
#include <iostream>

using namespace bptree;

int main() {
    // Test 1: Insert into empty leaf
    Node* leaf = new Node();
    leaf->isLeaf = true;
    new (&leaf->ptr2TreeOrData.dataPtr) std::vector<FILE*>;
    
    insert_into_sorted_leaf(leaf, 10, NULL);
    assert(leaf->keys.size() == 1);
    assert(leaf->keys[0] == 10);
    
    // Test 2: Insert at beginning
    insert_into_sorted_leaf(leaf, 5, NULL);
    assert(leaf->keys.size() == 2);
    assert(leaf->keys[0] == 5);
    assert(leaf->keys[1] == 10);
    
    // Test 3: Insert at end
    insert_into_sorted_leaf(leaf, 20, NULL);
    assert(leaf->keys.size() == 3);
    assert(leaf->keys[0] == 5);
    assert(leaf->keys[1] == 10);
    assert(leaf->keys[2] == 20);
    
    // Test 4: Insert in middle
    insert_into_sorted_leaf(leaf, 15, NULL);
    assert(leaf->keys.size() == 4);
    assert(leaf->keys[0] == 5);
    assert(leaf->keys[1] == 10);
    assert(leaf->keys[2] == 15);
    assert(leaf->keys[3] == 20);
    
    // Test 5: Insert another middle value
    insert_into_sorted_leaf(leaf, 12, NULL);
    assert(leaf->keys.size() == 5);
    assert(leaf->keys[0] == 5);
    assert(leaf->keys[1] == 10);
    assert(leaf->keys[2] == 12);
    assert(leaf->keys[3] == 15);
    assert(leaf->keys[4] == 20);
    
    std::cout << "All insert_into_sorted_leaf tests passed!" << std::endl;
    return 0;
}
