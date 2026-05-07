#include "../src/leveldb_functions.h"
#include <cassert>
#include <cstdint>
#include <string>

int main() {
    // Test 1: Value 1 → 1 byte [0x01]
    std::string s1;
    leveldb::PutVarint32(&s1, 1);
    assert(s1.size() == 1);
    assert(static_cast<uint8_t>(s1[0]) == 0x01);

    // Test 2: Value 127 → 1 byte [0x7F]
    std::string s2;
    leveldb::PutVarint32(&s2, 127);
    assert(s2.size() == 1);
    assert(static_cast<uint8_t>(s2[0]) == 0x7F);

    // Test 3: Value 128 → 2 bytes [0x80, 0x01]
    std::string s3;
    leveldb::PutVarint32(&s3, 128);
    assert(s3.size() == 2);
    assert(static_cast<uint8_t>(s3[0]) == 0x80);
    assert(static_cast<uint8_t>(s3[1]) == 0x01);

    // Test 4: Value 300 → 2 bytes [0xAC, 0x02]
    std::string s4;
    leveldb::PutVarint32(&s4, 300);
    assert(s4.size() == 2);
    assert(static_cast<uint8_t>(s4[0]) == 0xAC);
    assert(static_cast<uint8_t>(s4[1]) == 0x02);

    // Test 5: Value 0xFFFFFFFF → 5 bytes
    std::string s5;
    leveldb::PutVarint32(&s5, 0xFFFFFFFFu);
    assert(s5.size() == 5);

    // Test 6: Appends to existing content
    std::string s6 = "AB";
    leveldb::PutVarint32(&s6, 1);
    assert(s6.size() == 3);
    assert(s6[0] == 'A' && s6[1] == 'B');
    assert(static_cast<uint8_t>(s6[2]) == 0x01);

    // Test 7: Sequential values decode correctly
    std::string s7;
    leveldb::PutVarint32(&s7, 100);
    leveldb::PutVarint32(&s7, 200);
    leveldb::PutVarint32(&s7, 0xFFFFFFFFu);
    leveldb::Slice in(s7);
    uint32_t a, b, c;
    assert(leveldb::GetVarint32(&in, &a)); assert(a == 100);
    assert(leveldb::GetVarint32(&in, &b)); assert(b == 200);
    assert(leveldb::GetVarint32(&in, &c)); assert(c == 0xFFFFFFFFu);
    assert(in.size() == 0);

    return 0;
}
