#include "../src/leveldb_functions.h"
#include <cassert>
#include <cstring>
#include <cstdint>

int main() {
    char buf[10];

    // --- 1-byte: value 0 → [0x00] ---
    memset(buf, 0xAA, sizeof(buf));
    char* end = leveldb::EncodeVarint32(buf, 0);
    assert(end - buf == 1);
    assert(static_cast<uint8_t>(buf[0]) == 0x00);
    assert(static_cast<uint8_t>(buf[1]) == 0xAA);  // guard

    // --- 1-byte: value 127 → [0x7F] ---
    end = leveldb::EncodeVarint32(buf, 127);
    assert(end - buf == 1);
    assert(static_cast<uint8_t>(buf[0]) == 0x7F);

    // --- 2-byte: value 128 → [0x80, 0x01] ---
    end = leveldb::EncodeVarint32(buf, 128);
    assert(end - buf == 2);
    assert(static_cast<uint8_t>(buf[0]) == 0x80);
    assert(static_cast<uint8_t>(buf[1]) == 0x01);

    // --- 2-byte: value 300 → [0xAC, 0x02] ---
    end = leveldb::EncodeVarint32(buf, 300);
    assert(end - buf == 2);
    assert(static_cast<uint8_t>(buf[0]) == 0xAC);
    assert(static_cast<uint8_t>(buf[1]) == 0x02);

    // --- 3-byte: value 16384 → [0x80, 0x80, 0x01] ---
    end = leveldb::EncodeVarint32(buf, 16384);
    assert(end - buf == 3);
    assert(static_cast<uint8_t>(buf[0]) == 0x80);
    assert(static_cast<uint8_t>(buf[1]) == 0x80);
    assert(static_cast<uint8_t>(buf[2]) == 0x01);

    // --- 5-byte: 0x80000000 → [0x80, 0x80, 0x80, 0x80, 0x08] ---
    memset(buf, 0xAA, sizeof(buf));
    end = leveldb::EncodeVarint32(buf, 0x80000000u);
    assert(end - buf == 5);
    assert(static_cast<uint8_t>(buf[0]) == 0x80);
    assert(static_cast<uint8_t>(buf[1]) == 0x80);
    assert(static_cast<uint8_t>(buf[2]) == 0x80);
    assert(static_cast<uint8_t>(buf[3]) == 0x80);
    assert(static_cast<uint8_t>(buf[4]) == 0x08);
    assert(static_cast<uint8_t>(buf[5]) == 0xAA);  // guard

    // --- 5-byte: 0x7FFFFFFF → [0xFF, 0xFF, 0xFF, 0xFF, 0x07] ---
    memset(buf, 0xAA, sizeof(buf));
    end = leveldb::EncodeVarint32(buf, 0x7FFFFFFFu);
    assert(end - buf == 5);
    assert(static_cast<uint8_t>(buf[0]) == 0xFF);
    assert(static_cast<uint8_t>(buf[1]) == 0xFF);
    assert(static_cast<uint8_t>(buf[2]) == 0xFF);
    assert(static_cast<uint8_t>(buf[3]) == 0xFF);
    assert(static_cast<uint8_t>(buf[4]) == 0x07);
    assert(static_cast<uint8_t>(buf[5]) == 0xAA);  // guard

    // --- 5-byte: 0xFFFFFFFF — all continuation bytes have MSB set, last does not ---
    end = leveldb::EncodeVarint32(buf, 0xFFFFFFFFu);
    assert(end - buf == 5);
    for (int i = 0; i < 4; i++) assert(static_cast<uint8_t>(buf[i]) & 0x80);
    assert(!(static_cast<uint8_t>(buf[4]) & 0x80));

    // --- Fuzz: 1000 random round-trips ---
    uint32_t lcg = 0xDEADBEEFu;
    for (int i = 0; i < 1000; i++) {
        lcg = lcg * 1664525u + 1013904223u;
        memset(buf, 0xAA, sizeof(buf));
        char* e = leveldb::EncodeVarint32(buf, lcg);
        int len = e - buf;
        // Length must match VarintLength
        assert(len == leveldb::VarintLength(lcg));
        // All bytes except last must have MSB set
        for (int j = 0; j < len - 1; j++)
            assert(static_cast<uint8_t>(buf[j]) & 0x80);
        // Last byte must NOT have MSB set
        assert(!(static_cast<uint8_t>(buf[len - 1]) & 0x80));
        // Guard byte untouched
        assert(static_cast<uint8_t>(buf[len]) == 0xAA);
        // Round-trip
        uint32_t decoded;
        const char* r = leveldb::GetVarint32Ptr(buf, e, &decoded);
        assert(r == e);
        assert(decoded == lcg);
    }

    return 0;
}
