#include "../src/leveldb_functions.h"
#include <cassert>
#include <cstdint>

int main() {
    using namespace leveldb;

    // The formula: delta = (h >> 17) | (h << 15); return h + delta * probe_num

    // Test 1: probe 0 always returns h unchanged
    assert(BloomFilter_DoubleHash(12345u, 0) == 12345u);
    assert(BloomFilter_DoubleHash(0u, 0) == 0u);
    assert(BloomFilter_DoubleHash(0xFFFFFFFFu, 0) == 0xFFFFFFFFu);

    // Test 2: Pinned value for h=12345, probe=1
    uint32_t h = 12345u;
    uint32_t delta = (h >> 17) | (h << 15);
    assert(BloomFilter_DoubleHash(h, 1) == h + delta * 1);

    // Test 3: Pinned value for h=12345, probe=2
    assert(BloomFilter_DoubleHash(h, 2) == h + delta * 2);

    // Test 4: Pinned value for h=12345, probe=5
    assert(BloomFilter_DoubleHash(h, 5) == h + delta * 5);

    // Test 5: Linear spacing — consecutive probes differ by delta
    uint32_t h2 = 0xABCDEF01u;
    uint32_t delta2 = (h2 >> 17) | (h2 << 15);
    uint32_t v0 = BloomFilter_DoubleHash(h2, 0);
    uint32_t v1 = BloomFilter_DoubleHash(h2, 1);
    uint32_t v2 = BloomFilter_DoubleHash(h2, 2);
    assert(v1 - v0 == delta2);
    assert(v2 - v1 == delta2);

    // Test 6: Different h values produce different probe sequences
    assert(BloomFilter_DoubleHash(100u, 1) != BloomFilter_DoubleHash(200u, 1));

    return 0;
}
