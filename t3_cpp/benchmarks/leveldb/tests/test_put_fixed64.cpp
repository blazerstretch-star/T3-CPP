#include "../src/leveldb_functions.h"
#include <cassert>
#include <cstdint>
#include <string>

int main() {
    // Test 1: Appends exactly 8 bytes in little-endian order
    std::string s1;
    leveldb::PutFixed64(&s1, 0x123456789ABCDEF0ULL);
    assert(s1.size() == 8);
    assert(static_cast<uint8_t>(s1[0]) == 0xF0);
    assert(static_cast<uint8_t>(s1[1]) == 0xDE);
    assert(static_cast<uint8_t>(s1[2]) == 0xBC);
    assert(static_cast<uint8_t>(s1[3]) == 0x9A);
    assert(static_cast<uint8_t>(s1[4]) == 0x78);
    assert(static_cast<uint8_t>(s1[5]) == 0x56);
    assert(static_cast<uint8_t>(s1[6]) == 0x34);
    assert(static_cast<uint8_t>(s1[7]) == 0x12);

    // Test 2: Zero value
    std::string s2;
    leveldb::PutFixed64(&s2, 0);
    assert(s2.size() == 8);
    for (int i = 0; i < 8; i++) assert(static_cast<uint8_t>(s2[i]) == 0x00);

    // Test 3: Max value
    std::string s3;
    leveldb::PutFixed64(&s3, 0xFFFFFFFFFFFFFFFFULL);
    assert(s3.size() == 8);
    for (int i = 0; i < 8; i++) assert(static_cast<uint8_t>(s3[i]) == 0xFF);

    // Test 4: Appends to existing content
    std::string s4 = "XY";
    leveldb::PutFixed64(&s4, 1ULL);
    assert(s4.size() == 10);
    assert(s4[0] == 'X' && s4[1] == 'Y');
    assert(static_cast<uint8_t>(s4[2]) == 0x01);
    for (int i = 3; i < 10; i++) assert(static_cast<uint8_t>(s4[i]) == 0x00);

    // Test 5: Two sequential appends, round-trip decode
    std::string s5;
    leveldb::PutFixed64(&s5, 1000ULL);
    leveldb::PutFixed64(&s5, 0xDEADBEEFCAFEBABEULL);
    assert(s5.size() == 16);
    assert(leveldb::DecodeFixed64(s5.data()) == 1000ULL);
    assert(leveldb::DecodeFixed64(s5.data() + 8) == 0xDEADBEEFCAFEBABEULL);

    return 0;
}
