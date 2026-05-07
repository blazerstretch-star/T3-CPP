#include "../src/leveldb_functions.h"
#include <cassert>

int main() {
    // Test 1: No shared prefix
    leveldb::Slice a1("abc");
    leveldb::Slice b1("xyz");
    assert(leveldb::CalculateSharedPrefix(a1, b1) == 0);
    
    // Test 2: Full shared prefix
    leveldb::Slice a2("hello");
    leveldb::Slice b2("hello");
    assert(leveldb::CalculateSharedPrefix(a2, b2) == 5);
    
    // Test 3: Partial shared prefix
    leveldb::Slice a3("hello");
    leveldb::Slice b3("help");
    assert(leveldb::CalculateSharedPrefix(a3, b3) == 3);
    
    // Test 4: Different lengths
    leveldb::Slice a4("test");
    leveldb::Slice b4("testing");
    assert(leveldb::CalculateSharedPrefix(a4, b4) == 4);
    
    // Test 5: Empty strings
    leveldb::Slice a5("");
    leveldb::Slice b5("abc");
    assert(leveldb::CalculateSharedPrefix(a5, b5) == 0);
    
    return 0;
}
