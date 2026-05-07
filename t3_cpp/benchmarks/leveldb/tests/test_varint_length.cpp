#include "../src/leveldb_functions.h"
#include <cassert>
#include <cstdint>

int main() {
    // --- All 10 tier boundaries (exact spec values) ---

    // 1-byte: [0, 127]
    assert(leveldb::VarintLength(0) == 1);
    assert(leveldb::VarintLength(1) == 1);
    assert(leveldb::VarintLength(127) == 1);

    // 2-byte: [128, 16383]
    assert(leveldb::VarintLength(128) == 2);
    assert(leveldb::VarintLength(16383) == 2);

    // 3-byte: [16384, 2097151]
    assert(leveldb::VarintLength(16384) == 3);
    assert(leveldb::VarintLength(2097151) == 3);

    // 4-byte: [2097152, 268435455]
    assert(leveldb::VarintLength(2097152) == 4);
    assert(leveldb::VarintLength(268435455) == 4);

    // 5-byte: [268435456, 2^35-1]
    assert(leveldb::VarintLength(268435456) == 5);
    assert(leveldb::VarintLength(0xFFFFFFFFu) == 5);
    assert(leveldb::VarintLength(34359738367ULL) == 5);

    // 6-byte: [2^35, 2^42-1]
    assert(leveldb::VarintLength(34359738368ULL) == 6);
    assert(leveldb::VarintLength(0x800000000ULL) == 6);

    // 7-byte: [2^42, 2^49-1]
    assert(leveldb::VarintLength(1ULL << 42) == 7);
    assert(leveldb::VarintLength((1ULL << 49) - 1) == 7);

    // 8-byte: [2^49, 2^56-1]
    assert(leveldb::VarintLength(1ULL << 49) == 8);
    assert(leveldb::VarintLength((1ULL << 56) - 1) == 8);

    // 9-byte: [2^56, 2^63-1]
    assert(leveldb::VarintLength(1ULL << 56) == 9);
    assert(leveldb::VarintLength((1ULL << 63) - 1) == 9);

    // 10-byte: [2^63, 2^64-1]
    assert(leveldb::VarintLength(1ULL << 63) == 10);
    assert(leveldb::VarintLength(0xFFFFFFFFFFFFFFFFULL) == 10);

    // --- Monotonicity at every tier boundary ---
    uint64_t boundaries[] = {128, 16384, 2097152, 268435456,
                              34359738368ULL, 1ULL<<42, 1ULL<<49, 1ULL<<56, 1ULL<<63};
    for (uint64_t b : boundaries) {
        assert(leveldb::VarintLength(b) == leveldb::VarintLength(b - 1) + 1);
        assert(leveldb::VarintLength(b) < leveldb::VarintLength(b + 1) ||
               leveldb::VarintLength(b) == leveldb::VarintLength(b + 1));
    }

    // --- Consistency: VarintLength(v) == actual encoded byte count ---
    // This is the most important generalization check — any correct implementation
    // must produce a length that matches what EncodeVarint64 actually writes.
    char buf[10];
    uint64_t test_vals[] = {0, 1, 127, 128, 16383, 16384, 268435455u,
                            268435456u, 0xFFFFFFFFu, 1ULL<<35, 1ULL<<42,
                            1ULL<<49, 1ULL<<56, 1ULL<<63, 0xFFFFFFFFFFFFFFFFULL};
    for (uint64_t v : test_vals) {
        char* end = leveldb::EncodeVarint64(buf, v);
        assert((end - buf) == leveldb::VarintLength(v));
    }

    // --- Fuzz: 1000 random values — VarintLength matches actual encoding ---
    uint64_t lcg = 0xDEADBEEFCAFEBABEULL;
    for (int i = 0; i < 1000; i++) {
        lcg = lcg * 6364136223846793005ULL + 1442695040888963407ULL;
        char* end = leveldb::EncodeVarint64(buf, lcg);
        assert((end - buf) == leveldb::VarintLength(lcg));
        // Also verify VarintLength is in [1, 10]
        assert(leveldb::VarintLength(lcg) >= 1 && leveldb::VarintLength(lcg) <= 10);
    }

    return 0;
}
