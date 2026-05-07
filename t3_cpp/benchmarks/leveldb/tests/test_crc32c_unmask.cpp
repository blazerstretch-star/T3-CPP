#include "../src/leveldb_functions.h"
#include <cassert>
#include <cstdint>

int main() {
    // Test 1: Unmask(Mask(0)) == 0
    assert(leveldb::crc32c_Unmask(leveldb::crc32c_Mask(0)) == 0);

    // Test 2: Unmask(Mask(x)) == x for many values
    uint32_t vals[] = {1, 0x12345678u, 0xABCDEF00u, 0xFFFFFFFFu,
                       0x80000000u, 0x7FFFFFFFu, 0xDEADBEEFu};
    for (uint32_t v : vals) {
        assert(leveldb::crc32c_Unmask(leveldb::crc32c_Mask(v)) == v);
    }

    // Test 3: Pinned — Unmask(0xa282ead8) == 0 (since Mask(0) == 0xa282ead8)
    assert(leveldb::crc32c_Unmask(0xa282ead8u) == 0);

    // Test 4: Unmask is not identity
    assert(leveldb::crc32c_Unmask(0x12345678u) != 0x12345678u);

    // Test 5: Different masked values decode to different originals
    assert(leveldb::crc32c_Unmask(leveldb::crc32c_Mask(0x11111111u)) !=
           leveldb::crc32c_Unmask(leveldb::crc32c_Mask(0x22222222u)));

    return 0;
}
