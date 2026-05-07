#include "../src/leveldb_functions.h"
#include <cassert>
#include <cstdint>

int main() {
    char buf[10];

    // Test 1: Single-byte value 100
    char* p1 = leveldb::EncodeVarint64(buf, 100);
    uint64_t v1;
    const char* r1 = leveldb::GetVarint64Ptr(buf, p1, &v1);
    assert(r1 == p1);
    assert(v1 == 100);

    // Test 2: Two-byte value 128
    char* p2 = leveldb::EncodeVarint64(buf, 128);
    uint64_t v2;
    const char* r2 = leveldb::GetVarint64Ptr(buf, p2, &v2);
    assert(r2 == p2);
    assert(v2 == 128);

    // Test 3: Large value 0x123456789ABCDEF
    char* p3 = leveldb::EncodeVarint64(buf, 0x123456789ABCDEFULL);
    uint64_t v3;
    const char* r3 = leveldb::GetVarint64Ptr(buf, p3, &v3);
    assert(r3 == p3);
    assert(v3 == 0x123456789ABCDEFULL);

    // Test 4: Max uint64 — 10 bytes
    char* p4 = leveldb::EncodeVarint64(buf, 0xFFFFFFFFFFFFFFFFULL);
    uint64_t v4;
    const char* r4 = leveldb::GetVarint64Ptr(buf, p4, &v4);
    assert(r4 == p4);
    assert(v4 == 0xFFFFFFFFFFFFFFFFULL);

    // Test 5: Truncated buffer → nullptr
    char* p5 = leveldb::EncodeVarint64(buf, 0xFFFFFFFFFFFFFFFFULL);  // 10 bytes
    uint64_t v5;
    const char* r5 = leveldb::GetVarint64Ptr(buf, buf + 3, &v5);
    assert(r5 == nullptr);

    // Test 6: Empty buffer → nullptr
    uint64_t v6;
    const char* r6 = leveldb::GetVarint64Ptr(buf, buf, &v6);
    assert(r6 == nullptr);

    // Test 7: Sequential decode
    char multi[20];
    char* wp = leveldb::EncodeVarint64(multi, 0x123456789ABCDEFULL);
    leveldb::EncodeVarint64(wp, 42);
    uint64_t va, vb;
    const char* ra = leveldb::GetVarint64Ptr(multi, multi + 20, &va);
    assert(va == 0x123456789ABCDEFULL);
    const char* rb = leveldb::GetVarint64Ptr(ra, multi + 20, &vb);
    assert(vb == 42);

    return 0;
}
