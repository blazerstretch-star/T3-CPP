#include "../src/leveldb_functions.h"
#include <cassert>
#include <cstring>
#include <cstdint>
#include <string>

int main() {
    // --- Exact byte layout: user_key + LE64((seq<<8)|type) ---
    std::string r1;
    leveldb::AppendInternalKey(&r1, leveldb::Slice("mykey", 5), 100, 1);
    assert(r1.size() == 13);
    assert(memcmp(r1.data(), "mykey", 5) == 0);
    assert(leveldb::DecodeFixed64(r1.data() + 5) == ((uint64_t(100) << 8) | 1));

    // --- type=0 (deletion) ---
    std::string r2;
    leveldb::AppendInternalKey(&r2, leveldb::Slice("k", 1), 1, 0);
    assert(leveldb::DecodeFixed64(r2.data() + 1) == ((uint64_t(1) << 8) | 0));

    // --- Appends to existing content ---
    std::string r3 = "PREFIX";
    leveldb::AppendInternalKey(&r3, leveldb::Slice("ab", 2), 50, 1);
    assert(r3.size() == 16);
    assert(memcmp(r3.data(), "PREFIX", 6) == 0);
    assert(memcmp(r3.data() + 6, "ab", 2) == 0);

    // --- Large sequence number ---
    std::string r4;
    leveldb::AppendInternalKey(&r4, leveldb::Slice("x", 1), 0xFFFFFFFFFFFFULL, 1);
    assert(leveldb::DecodeFixed64(r4.data() + 1) == ((0xFFFFFFFFFFFFULL << 8) | 1));

    // --- Fuzz: 100 random (key_len, seq, type) triples ---
    uint32_t lcg = 0xDEADBEEFu;
    for (int i = 0; i < 100; i++) {
        lcg = lcg * 1664525u + 1013904223u;
        size_t klen = (lcg % 20) + 1;
        lcg = lcg * 1664525u + 1013904223u;
        uint64_t seq = lcg & 0x00FFFFFFFFFFFFFFull;  // 56-bit max
        uint8_t type = (lcg >> 24) & 0x01;           // 0 or 1 only

        // Build key bytes
        char kbuf[20];
        for (size_t j = 0; j < klen; j++) {
            lcg = lcg * 1664525u + 1013904223u;
            kbuf[j] = static_cast<char>(lcg & 0xFF);
        }

        std::string result;
        leveldb::AppendInternalKey(&result, leveldb::Slice(kbuf, klen), seq, type);

        // Size must be klen + 8
        assert(result.size() == klen + 8);
        // Key bytes preserved exactly
        assert(memcmp(result.data(), kbuf, klen) == 0);
        // Packed tag correct
        uint64_t packed = leveldb::DecodeFixed64(result.data() + klen);
        assert(packed == ((seq << 8) | type));

        // Round-trip via ParseInternalKey
        leveldb::Slice uk; uint64_t rseq; uint8_t rtype;
        assert(leveldb::ParseInternalKey(leveldb::Slice(result), &uk, &rseq, &rtype));
        assert(uk.size() == klen);
        assert(memcmp(uk.data(), kbuf, klen) == 0);
        assert(rseq == seq);
        assert(rtype == type);
    }

    return 0;
}
