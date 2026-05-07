#include "../src/leveldb_functions.h"
#include <cassert>
#include <cstring>
#include <cstdint>

// Reference implementation — identical algorithm to hash.cc
static uint32_t ref_hash(const char* data, size_t n, uint32_t seed) {
    const uint32_t m = 0xc6a4a793;
    const uint32_t r = 24;
    const char* limit = data + n;
    uint32_t h = seed ^ (static_cast<uint32_t>(n) * m);
    while (limit - data >= 4) {
        uint32_t w = static_cast<uint32_t>(static_cast<uint8_t>(data[0]))       |
                    (static_cast<uint32_t>(static_cast<uint8_t>(data[1])) << 8)  |
                    (static_cast<uint32_t>(static_cast<uint8_t>(data[2])) << 16) |
                    (static_cast<uint32_t>(static_cast<uint8_t>(data[3])) << 24);
        data += 4;
        h += w; h *= m; h ^= (h >> 16);
    }
    switch (limit - data) {
        case 3: h += static_cast<uint8_t>(data[2]) << 16; // fall through
        case 2: h += static_cast<uint8_t>(data[1]) << 8;  // fall through
        case 1: h += static_cast<uint8_t>(data[0]); h *= m; h ^= (h >> r); break;
    }
    return h;
}

int main() {
    // --- Pinned values for specific inputs ---
    assert(leveldb::Hash("",        0, 0)           == ref_hash("",        0, 0));
    assert(leveldb::Hash("a",       1, 0)           == ref_hash("a",       1, 0));
    assert(leveldb::Hash("hello",   5, 0)           == ref_hash("hello",   5, 0));
    assert(leveldb::Hash("hello",   5, 0xbc9f1d34)  == ref_hash("hello",   5, 0xbc9f1d34));
    assert(leveldb::Hash("abcdefgh",8, 0)           == ref_hash("abcdefgh",8, 0));
    assert(leveldb::Hash("abcde",   5, 0)           == ref_hash("abcde",   5, 0));

    // --- Seed sensitivity ---
    assert(leveldb::Hash("data", 4, 0) != leveldb::Hash("data", 4, 1));

    // --- Input sensitivity ---
    assert(leveldb::Hash("hello", 5, 0) != leveldb::Hash("world", 5, 0));

    // --- Determinism ---
    assert(leveldb::Hash("test", 4, 42) == leveldb::Hash("test", 4, 42));

    // --- Fuzz: 1000 random (data, seed) pairs against reference ---
    uint32_t lcg = 0xDEADBEEFu;
    char data[16];
    for (int i = 0; i < 1000; i++) {
        // Generate random data bytes
        for (int j = 0; j < 16; j++) {
            lcg = lcg * 1664525u + 1013904223u;
            data[j] = static_cast<char>(lcg & 0xFF);
        }
        uint32_t seed = lcg;
        // Test all lengths 0..15 with this data and seed
        for (size_t n = 0; n <= 15; n++) {
            assert(leveldb::Hash(data, n, seed) == ref_hash(data, n, seed));
        }
    }

    return 0;
}
