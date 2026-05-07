#include "../src/leveldb_functions.h"
#include <cassert>
#include <cstdint>

int main() {
    // BloomHash(key) == Hash(key.data(), key.size(), 0xbc9f1d34)
    // Pinned values computed independently from the MurmurHash-like algorithm.

    // Test 1: Pinned value for "hello" — 0xf795964e
    assert(leveldb::BloomHash(leveldb::Slice("hello", 5)) == 0xf795964eu);

    // Test 2: Pinned value for empty key — 0xbc9f1d34 (seed ^ (0 * m))
    assert(leveldb::BloomHash(leveldb::Slice("", 0)) == 0xbc9f1d34u);

    // Test 3: Pinned value for "key1" — 0x87cf4896
    assert(leveldb::BloomHash(leveldb::Slice("key1", 4)) == 0x87cf4896u);

    // Test 4: Pinned value for "test" — 0x0f27bca3
    assert(leveldb::BloomHash(leveldb::Slice("test", 4)) == 0x0f27bca3u);

    // Test 5: Uses seed 0xbc9f1d34 — result differs from Hash with seed 0
    // Hash("hello", 5, 0) = 0xc0eb4c52, BloomHash("hello") = 0xf795964e
    assert(leveldb::BloomHash(leveldb::Slice("hello", 5)) != 0xc0eb4c52u);

    // Test 6: Different keys produce different hashes
    // BloomHash("hello")=0xf795964e, BloomHash("world")=0x42c4e8fc
    assert(leveldb::BloomHash(leveldb::Slice("hello", 5)) !=
           leveldb::BloomHash(leveldb::Slice("world", 5)));

    // Test 7: Deterministic — same input always gives same output
    assert(leveldb::BloomHash(leveldb::Slice("test", 4)) ==
           leveldb::BloomHash(leveldb::Slice("test", 4)));

    return 0;
}
