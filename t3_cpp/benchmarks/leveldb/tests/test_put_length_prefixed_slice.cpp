#include "../src/leveldb_functions.h"
#include <cassert>
#include <cstring>
#include <cstdint>
#include <string>

int main() {
    // Test 1: Structure = varint32(len) + data bytes
    std::string s1;
    leveldb::PutLengthPrefixedSlice(&s1, leveldb::Slice("hello", 5));
    // Length prefix: 5 fits in 1 byte → [0x05]
    assert(s1.size() == 1 + 5);
    assert(static_cast<uint8_t>(s1[0]) == 0x05);
    assert(memcmp(s1.data() + 1, "hello", 5) == 0);

    // Test 2: Empty slice → varint32(0) + no data
    std::string s2;
    leveldb::PutLengthPrefixedSlice(&s2, leveldb::Slice("", 0));
    assert(s2.size() == 1);
    assert(static_cast<uint8_t>(s2[0]) == 0x00);

    // Test 3: Length >= 128 uses 2-byte varint prefix
    std::string long_data(200, 'x');
    std::string s3;
    leveldb::PutLengthPrefixedSlice(&s3, leveldb::Slice(long_data.data(), 200));
    // varint32(200) = [0xC8, 0x01] (2 bytes)
    assert(s3.size() == 2 + 200);
    assert(static_cast<uint8_t>(s3[0]) == 0xC8);
    assert(static_cast<uint8_t>(s3[1]) == 0x01);
    assert(memcmp(s3.data() + 2, long_data.data(), 200) == 0);

    // Test 4: Appends to existing content
    std::string s4 = "AB";
    leveldb::PutLengthPrefixedSlice(&s4, leveldb::Slice("xy", 2));
    assert(s4.size() == 5);
    assert(s4[0] == 'A' && s4[1] == 'B');
    assert(static_cast<uint8_t>(s4[2]) == 0x02);
    assert(s4[3] == 'x' && s4[4] == 'y');

    // Test 5: Round-trip with GetLengthPrefixedSlice
    std::string s5;
    leveldb::PutLengthPrefixedSlice(&s5, leveldb::Slice("abc", 3));
    leveldb::PutLengthPrefixedSlice(&s5, leveldb::Slice("defgh", 5));
    leveldb::Slice in(s5);
    leveldb::Slice r1, r2;
    assert(leveldb::GetLengthPrefixedSlice(&in, &r1));
    assert(r1.size() == 3);
    assert(memcmp(r1.data(), "abc", 3) == 0);
    assert(leveldb::GetLengthPrefixedSlice(&in, &r2));
    assert(r2.size() == 5);
    assert(memcmp(r2.data(), "defgh", 5) == 0);
    assert(in.size() == 0);

    return 0;
}
