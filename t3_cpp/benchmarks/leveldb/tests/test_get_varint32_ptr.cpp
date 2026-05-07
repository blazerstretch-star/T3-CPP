#include "../src/leveldb_functions.h"
#include <cassert>
#include <cstdint>

int main() {
    char buf[5];

    // Test 1: Single-byte value 1 → pointer advances by 1
    char* p1 = leveldb::EncodeVarint32(buf, 1);
    uint32_t v1;
    const char* r1 = leveldb::GetVarint32Ptr(buf, p1, &v1);
    assert(r1 == p1);
    assert(v1 == 1);

    // Test 2: Single-byte value 127
    char* p2 = leveldb::EncodeVarint32(buf, 127);
    uint32_t v2;
    const char* r2 = leveldb::GetVarint32Ptr(buf, p2, &v2);
    assert(r2 == p2);
    assert(v2 == 127);

    // Test 3: Two-byte value 128 → [0x80, 0x01]
    char* p3 = leveldb::EncodeVarint32(buf, 128);
    uint32_t v3;
    const char* r3 = leveldb::GetVarint32Ptr(buf, p3, &v3);
    assert(r3 == p3);
    assert(v3 == 128);

    // Test 4: Five-byte max value 0xFFFFFFFF
    char* p4 = leveldb::EncodeVarint32(buf, 0xFFFFFFFFu);
    uint32_t v4;
    const char* r4 = leveldb::GetVarint32Ptr(buf, p4, &v4);
    assert(r4 == p4);
    assert(v4 == 0xFFFFFFFFu);

    // Test 5: Truncated buffer → returns nullptr
    char* p5 = leveldb::EncodeVarint32(buf, 0xFFFFFFFFu);  // 5 bytes
    uint32_t v5;
    // Provide only 2 bytes of a 5-byte encoding → must fail
    const char* r5 = leveldb::GetVarint32Ptr(buf, buf + 2, &v5);
    assert(r5 == nullptr);

    // Test 6: Empty buffer → returns nullptr
    uint32_t v6;
    const char* r6 = leveldb::GetVarint32Ptr(buf, buf, &v6);
    assert(r6 == nullptr);

    // Test 7: Returned pointer is exactly past the encoded bytes
    char multi[10];
    char* wp = leveldb::EncodeVarint32(multi, 300);
    leveldb::EncodeVarint32(wp, 1);
    uint32_t va, vb;
    const char* ra = leveldb::GetVarint32Ptr(multi, multi + 10, &va);
    assert(va == 300);
    const char* rb = leveldb::GetVarint32Ptr(ra, multi + 10, &vb);
    assert(vb == 1);

    return 0;
}
