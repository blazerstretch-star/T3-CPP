#include "../src/leveldb_functions.h"
#include <cassert>
#include <cstdint>
#include <string>

int main() {
    // Test 1: Value 1 → 1 byte [0x01]
    std::string s1;
    leveldb::PutVarint64(&s1, 1);
    assert(s1.size() == 1);
    assert(static_cast<uint8_t>(s1[0]) == 0x01);

    // Test 2: Value 128 → 2 bytes [0x80, 0x01]
    std::string s2;
    leveldb::PutVarint64(&s2, 128);
    assert(s2.size() == 2);
    assert(static_cast<uint8_t>(s2[0]) == 0x80);
    assert(static_cast<uint8_t>(s2[1]) == 0x01);

    // Test 3: Max uint64 → 10 bytes
    std::string s3;
    leveldb::PutVarint64(&s3, 0xFFFFFFFFFFFFFFFFULL);
    assert(s3.size() == 10);

    // Test 4: Appends to existing content
    std::string s4 = "XY";
    leveldb::PutVarint64(&s4, 1);
    assert(s4.size() == 3);
    assert(s4[0] == 'X' && s4[1] == 'Y');
    assert(static_cast<uint8_t>(s4[2]) == 0x01);

    // Test 5: Sequential values decode correctly
    std::string s5;
    leveldb::PutVarint64(&s5, 1000);
    leveldb::PutVarint64(&s5, 0xFFFFFFFFFFFFFFFFULL);
    leveldb::PutVarint64(&s5, 0);
    leveldb::Slice in(s5);
    uint64_t a, b, c;
    assert(leveldb::GetVarint64(&in, &a)); assert(a == 1000);
    assert(leveldb::GetVarint64(&in, &b)); assert(b == 0xFFFFFFFFFFFFFFFFULL);
    assert(leveldb::GetVarint64(&in, &c)); assert(c == 0);
    assert(in.size() == 0);

    // Test 6: Size matches VarintLength
    uint64_t vals[] = {0, 127, 128, 16383, 16384, 0xFFFFFFFFFFFFFFFFULL};
    for (uint64_t v : vals) {
        std::string s;
        leveldb::PutVarint64(&s, v);
        assert((int)s.size() == leveldb::VarintLength(v));
    }

    return 0;
}
