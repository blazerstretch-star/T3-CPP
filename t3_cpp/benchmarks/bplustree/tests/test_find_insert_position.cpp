#include "../src/bptree.h"
#include <cassert>
#include <iostream>

using namespace bptree;

int main() {
    // Test 1: Empty vector
    std::vector<int> empty;
    assert(find_insert_position(empty, 5) == 0);
    
    // Test 2: Insert at beginning
    std::vector<int> keys1 = {10, 20, 30};
    assert(find_insert_position(keys1, 5) == 0);
    
    // Test 3: Insert in middle
    assert(find_insert_position(keys1, 15) == 1);
    assert(find_insert_position(keys1, 25) == 2);
    
    // Test 4: Insert at end
    assert(find_insert_position(keys1, 40) == 3);
    
    // Test 5: Duplicate key (upper_bound behavior)
    assert(find_insert_position(keys1, 20) == 2);
    assert(find_insert_position(keys1, 10) == 1);
    
    // Test 6: Single element
    std::vector<int> single = {10};
    assert(find_insert_position(single, 5) == 0);
    assert(find_insert_position(single, 15) == 1);
    assert(find_insert_position(single, 10) == 1);
    
    std::cout << "All find_insert_position tests passed!" << std::endl;
    return 0;
}
