#include "../src/leveldb_functions.h"
#include <cassert>
#include <cstdint>

int main() {
    // --- Pinned raw-byte inputs: no dependency on EncodeFixed64 ---
    char b0[8] = {0,0,0,0,0,0,0,0};
    assert(leveldb::DecodeFixed64(b0) == 0ULL);

    char bF[8];
    for (int i = 0; i < 8; i++) bF[i] = static_cast<char>(0xFF);
    assert(leveldb::DecodeFixed64(bF) == 0xFFFFFFFFFFFFFFFFULL);

    char b1[8] = {0x01, 0,0,0,0,0,0,0};
    assert(leveldb::DecodeFixed64(b1) == 0x0000000000000001ULL);

    char b80[8] = {0,0,0,0,0,0,0, static_cast<char>(0x80)};
    assert(leveldb::DecodeFixed64(b80) == 0x8000000000000000ULL);

    char b7F[8];
    for (int i = 0; i < 7; i++) b7F[i] = static_cast<char>(0xFF);
    b7F[7] = 0x7F;
    assert(leveldb::DecodeFixed64(b7F) == 0x7FFFFFFFFFFFFFFFULL);

    // 0x0807060504030201 — each byte is its own index
    char bidx[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    assert(leveldb::DecodeFixed64(bidx) == 0x0807060504030201ULL);

    // Byte order: 0x0000000100000000 → [0,0,0,0,1,0,0,0]
    char bord[8] = {0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
    assert(leveldb::DecodeFixed64(bord) == 0x0000000100000000ULL);

    // --- Each byte independently: only byte[i] = 0xFF, rest 0 ---
    for (int i = 0; i < 8; i++) {
        char single[8] = {0,0,0,0,0,0,0,0};
        single[i] = static_cast<char>(0xFF);
        uint64_t expected = static_cast<uint64_t>(0xFF) << (8 * i);
        assert(leveldb::DecodeFixed64(single) == expected);
    }

    // --- Bit-boundary sweep using raw bytes (no EncodeFixed64 dependency) ---
    for (int power = 0; power < 64; power++) {
        uint64_t v = 1ULL << power;
        char raw[8];
        for (int b = 0; b < 8; b++)
            raw[b] = static_cast<char>((v >> (8 * b)) & 0xFF);
        assert(leveldb::DecodeFixed64(raw) == v);
    }

    // --- Unaligned reads at all 8 offsets (0..7) ---
    for (int offset = 0; offset < 8; offset++) {
        char ubuf[16];
        for (int j = 0; j < 16; j++) ubuf[j] = static_cast<char>(0xCC);
        uint64_t v = 0x0807060504030201ULL;
        for (int b = 0; b < 8; b++)
            ubuf[offset + b] = static_cast<char>((v >> (8 * b)) & 0xFF);
        assert(leveldb::DecodeFixed64(ubuf + offset) == v);
    }

    // --- Fuzz: 1000 values via raw bytes (no EncodeFixed64 — avoids compensating bugs) ---
    uint64_t lcg = 0xDEADBEEFCAFEBABEULL;
    for (int i = 0; i < 1000; i++) {
        lcg = lcg * 6364136223846793005ULL + 1442695040888963407ULL;
        char raw[8];
        for (int b = 0; b < 8; b++)
            raw[b] = static_cast<char>((lcg >> (8 * b)) & 0xFF);
        assert(leveldb::DecodeFixed64(raw) == lcg);
    }

    return 0;
}
