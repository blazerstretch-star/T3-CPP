#include "../src/leveldb_functions.h"
#include <cassert>
#include <cstdint>

int main() {
    char buf[10];

    // Test 1: Single-byte decode
    char* p1 = leveldb::EncodeVarint64(buf, 1);
    leveldb::Slice s1(buf, p1 - buf);
    uint64_t v1;
    assert(leveldb::GetVarint64(&s1, &v1));
    assert(v1 == 1);
    assert(s1.size() == 0);

    // Test 2: Large value
    char* p2 = leveldb::EncodeVarint64(buf, 0x123456789ABCDEFULL);
    leveldb::Slice s2(buf, p2 - buf);
    uint64_t v2;
    assert(leveldb::GetVarint64(&s2, &v2));
    assert(v2 == 0x123456789ABCDEFULL);
    assert(s2.size() == 0);

    // Test 3: Max uint64
    char* p3 = leveldb::EncodeVarint64(buf, 0xFFFFFFFFFFFFFFFFULL);
    leveldb::Slice s3(buf, p3 - buf);
    uint64_t v3;
    assert(leveldb::GetVarint64(&s3, &v3));
    assert(v3 == 0xFFFFFFFFFFFFFFFFULL);
    assert(s3.size() == 0);

    // Test 4: Truncated → false
    char* p4 = leveldb::EncodeVarint64(buf, 0xFFFFFFFFFFFFFFFFULL);  // 10 bytes
    leveldb::Slice s4(buf, 3);
    uint64_t v4;
    assert(!leveldb::GetVarint64(&s4, &v4));

    // Test 5: Empty → false
    leveldb::Slice s5("", 0);
    uint64_t v5;
    assert(!leveldb::GetVarint64(&s5, &v5));

    // Test 6: Sequential values, slice advances correctly
    std::string seq;
    leveldb::PutVarint64(&seq, 1000);
    leveldb::PutVarint64(&seq, 0xFFFFFFFFFFFFFFFFULL);
    leveldb::PutVarint64(&seq, 0);
    leveldb::Slice s6(seq);
    uint64_t a, b, c;
    assert(leveldb::GetVarint64(&s6, &a)); assert(a == 1000);
    assert(leveldb::GetVarint64(&s6, &b)); assert(b == 0xFFFFFFFFFFFFFFFFULL);
    assert(leveldb::GetVarint64(&s6, &c)); assert(c == 0);
    assert(s6.size() == 0);

    return 0;
}
