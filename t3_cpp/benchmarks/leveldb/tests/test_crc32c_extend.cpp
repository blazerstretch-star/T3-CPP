#include "../src/leveldb_functions.h"
#include <cassert>
#include <cstring>
#include <cstdint>

int main() {
    // --- Standard test vector ---
    assert(leveldb::crc32c_Value("123456789", 9) == 0xe3069283u);

    // --- Incremental == full ---
    uint32_t crc1 = leveldb::crc32c_Value("hello", 5);
    assert(leveldb::crc32c_Extend(crc1, "world", 5) ==
           leveldb::crc32c_Value("helloworld", 10));

    // --- Extend with 0 bytes is identity ---
    uint32_t crc2 = leveldb::crc32c_Value("abc", 3);
    assert(leveldb::crc32c_Extend(crc2, "", 0) == crc2);

    // --- Extend from 0 == Value ---
    assert(leveldb::crc32c_Extend(0, "hello", 5) == leveldb::crc32c_Value("hello", 5));

    // --- Three-part incremental equals one-shot ---
    const char* s = "abcdefghij";
    uint32_t inc = leveldb::crc32c_Extend(0, s, 4);
    inc = leveldb::crc32c_Extend(inc, s + 4, 3);
    inc = leveldb::crc32c_Extend(inc, s + 7, 3);
    assert(inc == leveldb::crc32c_Value(s, 10));

    // --- Fuzz: 1000 random splits of random data ---
    uint32_t lcg = 0xDEADBEEFu;
    char data[32];
    for (int i = 0; i < 1000; i++) {
        // Fill data with pseudo-random bytes
        for (int j = 0; j < 32; j++) {
            lcg = lcg * 1664525u + 1013904223u;
            data[j] = static_cast<char>(lcg & 0xFF);
        }
        // Random split point in [0, 32]
        lcg = lcg * 1664525u + 1013904223u;
        size_t split = lcg % 33;

        uint32_t full = leveldb::crc32c_Value(data, 32);
        uint32_t part = leveldb::crc32c_Extend(0, data, split);
        part = leveldb::crc32c_Extend(part, data + split, 32 - split);
        assert(part == full);
    }

    return 0;
}
