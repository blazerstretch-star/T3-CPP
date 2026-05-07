#include "../src/leveldb_functions.h"
#include <cassert>
#include <cstdint>
#include <string>

int main() {
    // Test 1: Appends exactly 4 bytes in little-endian order
    std::string s1;
    leveldb::PutFixed32(&s1, 0x12345678u);
    assert(s1.size() == 4);
    assert(static_cast<uint8_t>(s1[0]) == 0x78);
    assert(static_cast<uint8_t>(s1[1]) == 0x56);
    assert(static_cast<uint8_t>(s1[2]) == 0x34);
    assert(static_cast<uint8_t>(s1[3]) == 0x12);

    // Test 2: Zero value
    std::string s2;
    leveldb::PutFixed32(&s2, 0);
    assert(s2.size() == 4);
    for (int i = 0; i < 4; i++) assert(static_cast<uint8_t>(s2[i]) == 0x00);

    // Test 3: Max value
    std::string s3;
    leveldb::PutFixed32(&s3, 0xFFFFFFFFu);
    assert(s3.size() == 4);
    for (int i = 0; i < 4; i++) assert(static_cast<uint8_t>(s3[i]) == 0xFF);

    // Test 4: Appends to existing content (does not overwrite)
    std::string s4 = "AB";
    leveldb::PutFixed32(&s4, 0x01020304u);
    assert(s4.size() == 6);
    assert(s4[0] == 'A' && s4[1] == 'B');
    assert(static_cast<uint8_t>(s4[2]) == 0x04);
    assert(static_cast<uint8_t>(s4[3]) == 0x03);
    assert(static_cast<uint8_t>(s4[4]) == 0x02);
    assert(static_cast<uint8_t>(s4[5]) == 0x01);

    // Test 5: Two sequential appends, each 4 bytes
    std::string s5;
    leveldb::PutFixed32(&s5, 100);
    leveldb::PutFixed32(&s5, 200);
    assert(s5.size() == 8);
    assert(leveldb::DecodeFixed32(s5.data()) == 100);
    assert(leveldb::DecodeFixed32(s5.data() + 4) == 200);

    return 0;
}
