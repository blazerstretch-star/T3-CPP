#include "../src/leveldb_functions.h"
#include <cassert>
#include <cstring>
#include <cstdint>

int main() {
    char buf[8];

    // --- Exact byte layout: all 6 critical edge values ---
    leveldb::EncodeFixed64(buf, 0ULL);
    for (int i = 0; i < 8; i++) assert(static_cast<uint8_t>(buf[i]) == 0x00);

    leveldb::EncodeFixed64(buf, 0xFFFFFFFFFFFFFFFFULL);
    for (int i = 0; i < 8; i++) assert(static_cast<uint8_t>(buf[i]) == 0xFF);

    leveldb::EncodeFixed64(buf, 0x0000000000000001ULL);
    assert(static_cast<uint8_t>(buf[0]) == 0x01);
    for (int i = 1; i < 8; i++) assert(static_cast<uint8_t>(buf[i]) == 0x00);

    leveldb::EncodeFixed64(buf, 0x8000000000000000ULL);
    for (int i = 0; i < 7; i++) assert(static_cast<uint8_t>(buf[i]) == 0x00);
    assert(static_cast<uint8_t>(buf[7]) == 0x80);

    leveldb::EncodeFixed64(buf, 0x7FFFFFFFFFFFFFFFULL);
    for (int i = 0; i < 7; i++) assert(static_cast<uint8_t>(buf[i]) == 0xFF);
    assert(static_cast<uint8_t>(buf[7]) == 0x7F);

    // 0x0807060504030201 — each byte is its own index (catches byte-swap bugs)
    leveldb::EncodeFixed64(buf, 0x0807060504030201ULL);
    assert(static_cast<uint8_t>(buf[0]) == 0x01);
    assert(static_cast<uint8_t>(buf[1]) == 0x02);
    assert(static_cast<uint8_t>(buf[2]) == 0x03);
    assert(static_cast<uint8_t>(buf[3]) == 0x04);
    assert(static_cast<uint8_t>(buf[4]) == 0x05);
    assert(static_cast<uint8_t>(buf[5]) == 0x06);
    assert(static_cast<uint8_t>(buf[6]) == 0x07);
    assert(static_cast<uint8_t>(buf[7]) == 0x08);

    leveldb::EncodeFixed64(buf, 0x123456789ABCDEF0ULL);
    assert(static_cast<uint8_t>(buf[0]) == 0xF0);
    assert(static_cast<uint8_t>(buf[1]) == 0xDE);
    assert(static_cast<uint8_t>(buf[2]) == 0xBC);
    assert(static_cast<uint8_t>(buf[3]) == 0x9A);
    assert(static_cast<uint8_t>(buf[4]) == 0x78);
    assert(static_cast<uint8_t>(buf[5]) == 0x56);
    assert(static_cast<uint8_t>(buf[6]) == 0x34);
    assert(static_cast<uint8_t>(buf[7]) == 0x12);

    // --- Bit-boundary sweep: for each power bit, test (v-1, v, v+1) ---
    for (int power = 1; power < 64; power++) {
        uint64_t v = 1ULL << power;
        uint64_t cases[] = {v - 1, v, v + 1};
        for (uint64_t c : cases) {
            char fb[8];
            leveldb::EncodeFixed64(fb, c);
            for (int b = 0; b < 8; b++)
                assert(static_cast<uint8_t>(fb[b]) == ((c >> (8 * b)) & 0xFF));
        }
    }

    // --- Guard bytes: no write beyond 8 bytes ---
    char guarded[10];
    memset(guarded, 0x55, sizeof(guarded));
    leveldb::EncodeFixed64(guarded, 0x123456789ABCDEF0ULL);
    assert(static_cast<uint8_t>(guarded[8]) == 0x55);
    assert(static_cast<uint8_t>(guarded[9]) == 0x55);

    // --- Reused buffer: second write fully overwrites first, no residue ---
    char rbuf[8];
    leveldb::EncodeFixed64(rbuf, 0xFFFFFFFFFFFFFFFFULL);
    leveldb::EncodeFixed64(rbuf, 0x0000000000000000ULL);
    for (int i = 0; i < 8; i++) assert(static_cast<uint8_t>(rbuf[i]) == 0x00);

    leveldb::EncodeFixed64(rbuf, 0x0000000000000000ULL);
    leveldb::EncodeFixed64(rbuf, 0xFFFFFFFFFFFFFFFFULL);
    for (int i = 0; i < 8; i++) assert(static_cast<uint8_t>(rbuf[i]) == 0xFF);

    // --- Consecutive writes: second does not corrupt first ---
    char buf16[16];
    leveldb::EncodeFixed64(buf16,     0xAAAAAAAAAAAAAAAAULL);
    leveldb::EncodeFixed64(buf16 + 8, 0xBBBBBBBBBBBBBBBBULL);
    assert(leveldb::DecodeFixed64(buf16)     == 0xAAAAAAAAAAAAAAAAULL);
    assert(leveldb::DecodeFixed64(buf16 + 8) == 0xBBBBBBBBBBBBBBBBULL);

    // --- Unaligned writes at all 8 offsets (0..7) ---
    for (int offset = 0; offset < 8; offset++) {
        char ubuf[16];
        memset(ubuf, 0xCC, sizeof(ubuf));
        uint64_t v = 0x0807060504030201ULL;
        leveldb::EncodeFixed64(ubuf + offset, v);
        // Verify all 8 written bytes
        for (int b = 0; b < 8; b++)
            assert(static_cast<uint8_t>(ubuf[offset + b]) == (b + 1));
        // Guard bytes before
        for (int j = 0; j < offset; j++)
            assert(static_cast<uint8_t>(ubuf[j]) == 0xCC);
        // Guard bytes after
        for (int j = offset + 8; j < 16; j++)
            assert(static_cast<uint8_t>(ubuf[j]) == 0xCC);
    }

    // --- Fuzz: 1000 random values, ALL 8 bytes checked ---
    uint64_t lcg = 0xDEADBEEFCAFEBABEULL;
    for (int i = 0; i < 1000; i++) {
        lcg = lcg * 6364136223846793005ULL + 1442695040888963407ULL;
        char fbuf[10];
        memset(fbuf, 0x55, sizeof(fbuf));
        leveldb::EncodeFixed64(fbuf, lcg);
        // All 8 bytes must match expected little-endian
        for (int b = 0; b < 8; b++)
            assert(static_cast<uint8_t>(fbuf[b]) == ((lcg >> (8 * b)) & 0xFF));
        // Guard bytes untouched
        assert(static_cast<uint8_t>(fbuf[8]) == 0x55);
        assert(static_cast<uint8_t>(fbuf[9]) == 0x55);
        // Round-trip
        assert(leveldb::DecodeFixed64(fbuf) == lcg);
    }

    return 0;
}
