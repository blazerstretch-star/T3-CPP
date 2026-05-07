#include "../src/leveldb_functions.h"
#include <cassert>

int main() {
    uint32_t seed1 = leveldb::RandomSeed();
    uint32_t seed2 = leveldb::RandomSeed();
    assert(seed1 == seed2);  // Stub implementation returns constant
    assert(seed1 == 0xdeadbeef);
    
    return 0;
}
