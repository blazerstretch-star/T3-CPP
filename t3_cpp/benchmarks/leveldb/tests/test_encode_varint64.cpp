#include "../src/leveldb_functions.h"
#include <cassert>
#include <cstring>
#include <cstdint>

int main() {
    char buf[11];  // 10 max + 1 guard

    // --- 1-byte: 0 → [0x00] ---
    memset(buf, 0xAA, sizeof(buf));
    char* end = leveldb::EncodeVarint64(buf, 0);
    assert(end - buf == 1);
    assert(static_cast<uint8_t>(buf[0]) == 0x00);
    assert(static_cast<uint8_t>(buf[1]) == 0xAA);  // guard

    // --- 1-byte: 127 → [0x7F] ---
    end = leveldb::EncodeVarint64(buf, 127);
    assert(end - buf == 1);
    assert(static_cast<uint8_t>(buf[0]) == 0x7F);

    // --- 2-byte: 128 → [0x80, 0x01] ---
    end = leveldb::EncodeVarint64(buf, 128);
    assert(end - buf == 2);
    assert(static_cast<uint8_t>(buf[0]) == 0x80);
    assert(static_cast<uint8_t>(buf[1]) == 0x01);

    // --- 2-byte: 300 → [0xAC, 0x02] ---
    end = leveldb::EncodeVarint64(buf, 300);
    assert(end - buf == 2);
    assert(static_cast<uint8_t>(buf[0]) == 0xAC);
    assert(static_cast<uint8_t>(buf[1]) == 0x02);

    // --- 10-byte: max uint64 — all 9 continuation bytes have MSB set, last does not ---
    memset(buf, 0xAA, sizeof(buf));
    end = leveldb::EncodeVarint64(buf, 0xFFFFFFFFFFFFFFFFULL);
    assert(end - buf == 10);
    for (int i = 0; i < 9; i++) assert(static_cast<uint8_t>(buf[i]) & 0x80);
    assert(!(static_cast<uint8_t>(buf[9]) & 0x80));
    assert(static_cast<uint8_t>(buf[10]) == 0xAA);  // guard

    // --- Length matches VarintLength for boundary values ---
    uint64_t boundaries[] = {0, 127, 128, 16383, 16384, 0xFFFFFFFFULL,
                              0x123456789ABCDEFULL, 0xFFFFFFFFFFFFFFFFULL};
    for (uint64_t v : boundaries) {
        char* e = leveldb::EncodeVarint64(buf, v);
        assert((e - buf) == leveldb::VarintLength(v));
    }

    // --- Fuzz: 1000 random round-trips ---
    uint64_t lcg = 0xDEADBEEFCAFEBABEULL;
    for (int i = 0; i < 1000; i++) {
        lcg = lcg * 6364136223846793005ULL + 1442695040888963407ULL;
        memset(buf, 0xAA, sizeof(buf));
        char* e = leveldb::EncodeVarint64(buf, lcg);
        int len = e - buf;
        assert(len == leveldb::VarintLength(lcg));
        // All bytes except last must have MSB set
        for (int j = 0; j < len - 1; j++)
            assert(static_cast<uint8_t>(buf[j]) & 0x80);
        // Last byte must NOT have MSB set
        assert(!(static_cast<uint8_t>(buf[len - 1]) & 0x80));
        // Guard byte untouched
        assert(static_cast<uint8_t>(buf[len]) == 0xAA);
        // Round-trip
        uint64_t decoded;
        const char* r = leveldb::GetVarint64Ptr(buf, e, &decoded);
        assert(r == e);
        assert(decoded == lcg);
    }

    return 0;
}
