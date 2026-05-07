#include "../src/leveldb_functions.h"
#include <cassert>
#include <cstdint>

int main() {
    char buf[5];

    // Test 1: Single-byte decode, slice advances to empty
    char* p1 = leveldb::EncodeVarint32(buf, 1);
    leveldb::Slice s1(buf, p1 - buf);
    uint32_t v1;
    assert(leveldb::GetVarint32(&s1, &v1));
    assert(v1 == 1);
    assert(s1.size() == 0);

    // Test 2: Multi-byte decode, slice advances correctly
    char* p2 = leveldb::EncodeVarint32(buf, 12345);
    leveldb::Slice s2(buf, p2 - buf);
    uint32_t v2;
    assert(leveldb::GetVarint32(&s2, &v2));
    assert(v2 == 12345);
    assert(s2.size() == 0);

    // Test 3: Large value 0xFFFFFFFF
    char* p3 = leveldb::EncodeVarint32(buf, 0xFFFFFFFFu);
    leveldb::Slice s3(buf, p3 - buf);
    uint32_t v3;
    assert(leveldb::GetVarint32(&s3, &v3));
    assert(v3 == 0xFFFFFFFFu);
    assert(s3.size() == 0);

    // Test 4: Truncated input → returns false
    char* p4 = leveldb::EncodeVarint32(buf, 0xFFFFFFFFu);  // 5 bytes
    leveldb::Slice s4(buf, 2);  // only 2 bytes available
    uint32_t v4;
    assert(!leveldb::GetVarint32(&s4, &v4));

    // Test 5: Empty slice → returns false
    leveldb::Slice s5("", 0);
    uint32_t v5;
    assert(!leveldb::GetVarint32(&s5, &v5));

    // Test 6: Two sequential values in one buffer, slice advances between them
    std::string seq;
    leveldb::PutVarint32(&seq, 100);
    leveldb::PutVarint32(&seq, 200);
    leveldb::PutVarint32(&seq, 0xFFFFFFFFu);
    leveldb::Slice s6(seq);
    uint32_t a, b, c;
    assert(leveldb::GetVarint32(&s6, &a)); assert(a == 100);
    assert(leveldb::GetVarint32(&s6, &b)); assert(b == 200);
    assert(leveldb::GetVarint32(&s6, &c)); assert(c == 0xFFFFFFFFu);
    assert(s6.size() == 0);

    // Test 7: Slice data pointer advances (not just size)
    char* p7 = leveldb::EncodeVarint32(buf, 42);
    leveldb::Slice s7(buf, p7 - buf + 1);  // 1 extra byte after
    uint32_t v7;
    assert(leveldb::GetVarint32(&s7, &v7));
    assert(v7 == 42);
    assert(s7.size() == 1);  // 1 byte remaining

    return 0;
}
