#include "../src/leveldb_functions.h"
#include <cassert>
#include <cstring>
#include <cstdint>

int main() {
    // --- Known CRC32C test vectors (standard, implementation-agnostic) ---

    // Standard test vector: "123456789" = 0xe3069283
    assert(leveldb::crc32c_Value("123456789", 9) == 0xe3069283u);

    // Empty string: Extend(0,"",0) = 0 XOR 0xFFFFFFFF XOR 0xFFFFFFFF = 0
    assert(leveldb::crc32c_Value("", 0) == 0u);

    // Single zero byte: CRC32C of {0x00} = 0x527d5351
    assert(leveldb::crc32c_Value("\x00", 1) == 0x527d5351u);

    // Single 0xFF byte: CRC32C of {0xFF} = 0xff000000
    assert(leveldb::crc32c_Value("\xff", 1) == 0xff000000u);

    // "a" = 0xc1d04330
    assert(leveldb::crc32c_Value("a", 1) == 0xc1d04330u);

    // "abc" = 0x364b3fb7
    assert(leveldb::crc32c_Value("abc", 3) == 0x364b3fb7u);

    // --- Behavioral properties (implementation-agnostic) ---

    // Deterministic
    assert(leveldb::crc32c_Value("hello", 5) == leveldb::crc32c_Value("hello", 5));

    // Different inputs → different CRCs
    assert(leveldb::crc32c_Value("hello", 5) != leveldb::crc32c_Value("world", 5));

    // Value == Extend(0, data, n)
    assert(leveldb::crc32c_Value("abc", 3) == leveldb::crc32c_Extend(0, "abc", 3));

    // Length sensitivity: "abc" != "ab"
    assert(leveldb::crc32c_Value("abc", 3) != leveldb::crc32c_Value("abc", 2));

    // Content sensitivity: "abc" != "abd"
    assert(leveldb::crc32c_Value("abc", 3) != leveldb::crc32c_Value("abd", 3));

    // --- Fuzz: 1000 random inputs — Value == Extend(0, data, n) ---
    uint32_t lcg = 0xDEADBEEFu;
    char data[32];
    for (int i = 0; i < 1000; i++) {
        for (int j = 0; j < 32; j++) {
            lcg = lcg * 1664525u + 1013904223u;
            data[j] = static_cast<char>(lcg & 0xFF);
        }
        lcg = lcg * 1664525u + 1013904223u;
        size_t n = (lcg % 32) + 1;
        assert(leveldb::crc32c_Value(data, n) ==
               leveldb::crc32c_Extend(0, data, n));
    }

    return 0;
}
