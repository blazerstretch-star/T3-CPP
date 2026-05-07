#include "../src/leveldb_functions.h"
#include <cassert>
#include <cstring>
#include <cstdint>

int main() {
    char buf[4];

    // --- Exact byte layout: all 6 critical edge values ---
    leveldb::EncodeFixed32(buf, 0x00000000u);
    assert(static_cast<uint8_t>(buf[0]) == 0x00);
    assert(static_cast<uint8_t>(buf[1]) == 0x00);
    assert(static_cast<uint8_t>(buf[2]) == 0x00);
    assert(static_cast<uint8_t>(buf[3]) == 0x00);

    leveldb::EncodeFixed32(buf, 0xFFFFFFFFu);
    assert(static_cast<uint8_t>(buf[0]) == 0xFF);
    assert(static_cast<uint8_t>(buf[1]) == 0xFF);
    assert(static_cast<uint8_t>(buf[2]) == 0xFF);
    assert(static_cast<uint8_t>(buf[3]) == 0xFF);

    leveldb::EncodeFixed32(buf, 0x00000001u);
    assert(static_cast<uint8_t>(buf[0]) == 0x01);
    assert(static_cast<uint8_t>(buf[1]) == 0x00);
    assert(static_cast<uint8_t>(buf[2]) == 0x00);
    assert(static_cast<uint8_t>(buf[3]) == 0x00);

    leveldb::EncodeFixed32(buf, 0x80000000u);
    assert(static_cast<uint8_t>(buf[0]) == 0x00);
    assert(static_cast<uint8_t>(buf[1]) == 0x00);
    assert(static_cast<uint8_t>(buf[2]) == 0x00);
    assert(static_cast<uint8_t>(buf[3]) == 0x80);

    leveldb::EncodeFixed32(buf, 0x7FFFFFFFu);
    assert(static_cast<uint8_t>(buf[0]) == 0xFF);
    assert(static_cast<uint8_t>(buf[1]) == 0xFF);
    assert(static_cast<uint8_t>(buf[2]) == 0xFF);
    assert(static_cast<uint8_t>(buf[3]) == 0x7F);

    leveldb::EncodeFixed32(buf, 0x12345678u);
    assert(static_cast<uint8_t>(buf[0]) == 0x78);
    assert(static_cast<uint8_t>(buf[1]) == 0x56);
    assert(static_cast<uint8_t>(buf[2]) == 0x34);
    assert(static_cast<uint8_t>(buf[3]) == 0x12);

    // --- Bit-boundary sweep: for each power bit, test (v-1, v, v+1) ---
    for (int power = 1; power < 32; power++) {
        uint32_t v = 1u << power;
        uint32_t cases[] = {v - 1, v, v + 1};
        for (uint32_t c : cases) {
            char fb[4];
            leveldb::EncodeFixed32(fb, c);
            // All 4 bytes must match the expected little-endian layout
            assert(static_cast<uint8_t>(fb[0]) == (c & 0xFF));
            assert(static_cast<uint8_t>(fb[1]) == ((c >> 8) & 0xFF));
            assert(static_cast<uint8_t>(fb[2]) == ((c >> 16) & 0xFF));
            assert(static_cast<uint8_t>(fb[3]) == ((c >> 24) & 0xFF));
        }
    }

    // --- Sequential range: first 100000 values ---
    for (uint32_t v = 0; v < 100000u; v++) {
        char fb[4];
        leveldb::EncodeFixed32(fb, v);
        assert(static_cast<uint8_t>(fb[0]) == (v & 0xFF));
        assert(static_cast<uint8_t>(fb[1]) == ((v >> 8) & 0xFF));
        assert(static_cast<uint8_t>(fb[2]) == ((v >> 16) & 0xFF));
        assert(static_cast<uint8_t>(fb[3]) == ((v >> 24) & 0xFF));
    }

    // --- Guard bytes: no write beyond 4 bytes ---
    char guarded[6];
    memset(guarded, 0x55, sizeof(guarded));
    leveldb::EncodeFixed32(guarded, 0xDEADBEEFu);
    assert(static_cast<uint8_t>(guarded[4]) == 0x55);
    assert(static_cast<uint8_t>(guarded[5]) == 0x55);

    // --- Reused buffer: second write fully overwrites first, no residue ---
    char rbuf[4];
    leveldb::EncodeFixed32(rbuf, 0xFFFFFFFFu);
    leveldb::EncodeFixed32(rbuf, 0x00000000u);
    for (int i = 0; i < 4; i++) assert(static_cast<uint8_t>(rbuf[i]) == 0x00);

    leveldb::EncodeFixed32(rbuf, 0x00000000u);
    leveldb::EncodeFixed32(rbuf, 0xFFFFFFFFu);
    for (int i = 0; i < 4; i++) assert(static_cast<uint8_t>(rbuf[i]) == 0xFF);

    // --- Consecutive writes: second does not corrupt first ---
    char buf8[8];
    leveldb::EncodeFixed32(buf8,     0xAAAAAAAAu);
    leveldb::EncodeFixed32(buf8 + 4, 0xBBBBBBBBu);
    assert(leveldb::DecodeFixed32(buf8)     == 0xAAAAAAAAu);
    assert(leveldb::DecodeFixed32(buf8 + 4) == 0xBBBBBBBBu);

    // --- Unaligned writes at all valid offsets (0..3) ---
    for (int offset = 0; offset < 4; offset++) {
        char ubuf[8];
        memset(ubuf, 0xCC, sizeof(ubuf));
        uint32_t v = 0x12345678u;
        leveldb::EncodeFixed32(ubuf + offset, v);
        // Verify all 4 written bytes
        assert(static_cast<uint8_t>(ubuf[offset + 0]) == 0x78);
        assert(static_cast<uint8_t>(ubuf[offset + 1]) == 0x56);
        assert(static_cast<uint8_t>(ubuf[offset + 2]) == 0x34);
        assert(static_cast<uint8_t>(ubuf[offset + 3]) == 0x12);
        // Verify guard bytes before and after
        for (int j = 0; j < offset; j++)
            assert(static_cast<uint8_t>(ubuf[j]) == 0xCC);
        for (int j = offset + 4; j < 8; j++)
            assert(static_cast<uint8_t>(ubuf[j]) == 0xCC);
    }

    // --- Fuzz: 1000 random values, ALL 4 bytes checked ---
    uint32_t lcg = 0xDEADBEEFu;
    for (int i = 0; i < 1000; i++) {
        lcg = lcg * 1664525u + 1013904223u;
        char fbuf[6];
        memset(fbuf, 0x55, sizeof(fbuf));
        leveldb::EncodeFixed32(fbuf, lcg);
        // All 4 bytes must match expected little-endian
        assert(static_cast<uint8_t>(fbuf[0]) == (lcg & 0xFF));
        assert(static_cast<uint8_t>(fbuf[1]) == ((lcg >> 8) & 0xFF));
        assert(static_cast<uint8_t>(fbuf[2]) == ((lcg >> 16) & 0xFF));
        assert(static_cast<uint8_t>(fbuf[3]) == ((lcg >> 24) & 0xFF));
        // Guard bytes untouched
        assert(static_cast<uint8_t>(fbuf[4]) == 0x55);
        assert(static_cast<uint8_t>(fbuf[5]) == 0x55);
        // Round-trip
        assert(leveldb::DecodeFixed32(fbuf) == lcg);
    }

    return 0;
}
