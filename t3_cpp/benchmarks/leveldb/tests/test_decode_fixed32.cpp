#include "../src/leveldb_functions.h"
#include <cassert>
#include <cstdint>

int main() {
    // --- Pinned raw-byte inputs: no dependency on EncodeFixed32 ---
    char b0[4]  = {0x00, 0x00, 0x00, 0x00};
    assert(leveldb::DecodeFixed32(b0) == 0x00000000u);

    char bF[4]  = {static_cast<char>(0xFF), static_cast<char>(0xFF),
                   static_cast<char>(0xFF), static_cast<char>(0xFF)};
    assert(leveldb::DecodeFixed32(bF) == 0xFFFFFFFFu);

    char b1[4]  = {0x01, 0x00, 0x00, 0x00};
    assert(leveldb::DecodeFixed32(b1) == 0x00000001u);

    char b80[4] = {0x00, 0x00, 0x00, static_cast<char>(0x80)};
    assert(leveldb::DecodeFixed32(b80) == 0x80000000u);

    char b7F[4] = {static_cast<char>(0xFF), static_cast<char>(0xFF),
                   static_cast<char>(0xFF), 0x7F};
    assert(leveldb::DecodeFixed32(b7F) == 0x7FFFFFFFu);

    char bm[4]  = {0x78, 0x56, 0x34, 0x12};
    assert(leveldb::DecodeFixed32(bm) == 0x12345678u);

    // Byte order: value 256 = [0x00, 0x01, 0x00, 0x00]
    char b256[4] = {0x00, 0x01, 0x00, 0x00};
    assert(leveldb::DecodeFixed32(b256) == 256u);

    // Each byte independently: only byte[i] set
    for (int i = 0; i < 4; i++) {
        char single[4] = {0, 0, 0, 0};
        single[i] = static_cast<char>(0xFF);
        uint32_t expected = 0xFFu << (8 * i);
        assert(leveldb::DecodeFixed32(single) == expected);
    }

    // --- Bit-boundary sweep using raw bytes (no EncodeFixed32 dependency) ---
    for (int power = 0; power < 32; power++) {
        uint32_t v = 1u << power;
        char raw[4];
        raw[0] = static_cast<char>(v & 0xFF);
        raw[1] = static_cast<char>((v >> 8) & 0xFF);
        raw[2] = static_cast<char>((v >> 16) & 0xFF);
        raw[3] = static_cast<char>((v >> 24) & 0xFF);
        assert(leveldb::DecodeFixed32(raw) == v);
    }

    // --- Sequential range: first 100000 values via raw bytes ---
    for (uint32_t v = 0; v < 100000u; v++) {
        char raw[4];
        raw[0] = static_cast<char>(v & 0xFF);
        raw[1] = static_cast<char>((v >> 8) & 0xFF);
        raw[2] = static_cast<char>((v >> 16) & 0xFF);
        raw[3] = static_cast<char>((v >> 24) & 0xFF);
        assert(leveldb::DecodeFixed32(raw) == v);
    }

    // --- Unaligned reads at all valid offsets (0..3) ---
    for (int offset = 0; offset < 4; offset++) {
        char ubuf[8];
        for (int j = 0; j < 8; j++) ubuf[j] = static_cast<char>(0xCC);
        uint32_t v = 0x12345678u;
        ubuf[offset + 0] = 0x78;
        ubuf[offset + 1] = 0x56;
        ubuf[offset + 2] = 0x34;
        ubuf[offset + 3] = 0x12;
        assert(leveldb::DecodeFixed32(ubuf + offset) == v);
    }

    // --- Fuzz: 1000 values via raw bytes (no EncodeFixed32 — avoids compensating bugs) ---
    uint32_t lcg = 0xDEADBEEFu;
    for (int i = 0; i < 1000; i++) {
        lcg = lcg * 1664525u + 1013904223u;
        char raw[4];
        raw[0] = static_cast<char>(lcg & 0xFF);
        raw[1] = static_cast<char>((lcg >> 8) & 0xFF);
        raw[2] = static_cast<char>((lcg >> 16) & 0xFF);
        raw[3] = static_cast<char>((lcg >> 24) & 0xFF);
        assert(leveldb::DecodeFixed32(raw) == lcg);
    }

    return 0;
}
