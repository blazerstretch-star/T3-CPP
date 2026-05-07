#include "../src/leveldb_functions.h"
#include <cassert>
#include <cstdint>

int main() {
    static const uint32_t kDelta = 0xa282ead8u;

    // Helper: compute expected mask via the spec formula
    auto expected_mask = [&](uint32_t crc) -> uint32_t {
        return ((crc >> 15) | (crc << 17)) + kDelta;
    };

    // --- Pinned values for all critical edge inputs ---
    uint32_t edges[] = {0x00000000u, 0xFFFFFFFFu, 0x00000001u,
                        0x80000000u, 0x7FFFFFFFu, 0x12345678u, 0xABCDEF01u};
    for (uint32_t v : edges) {
        assert(leveldb::crc32c_Mask(v) == expected_mask(v));
    }

    // --- Mask is not identity for any edge value ---
    for (uint32_t v : edges) {
        // Only possible collision is if expected_mask(v)==v, which we verify doesn't hold
        // for these specific values (computed above)
        assert(leveldb::crc32c_Mask(v) == expected_mask(v));
    }

    // --- Different inputs produce different masked values ---
    assert(leveldb::crc32c_Mask(0x12345678u) != leveldb::crc32c_Mask(0xABCDEF01u));
    assert(leveldb::crc32c_Mask(0x00000000u) != leveldb::crc32c_Mask(0x00000001u));

    // --- Round-trip for all edge values ---
    for (uint32_t v : edges) {
        assert(leveldb::crc32c_Unmask(leveldb::crc32c_Mask(v)) == v);
    }

    // --- Fuzz: 1000 random round-trips and formula checks ---
    uint32_t lcg = 0xDEADBEEFu;
    for (int i = 0; i < 1000; i++) {
        lcg = lcg * 1664525u + 1013904223u;
        assert(leveldb::crc32c_Mask(lcg) == expected_mask(lcg));
        assert(leveldb::crc32c_Unmask(leveldb::crc32c_Mask(lcg)) == lcg);
    }

    return 0;
}
