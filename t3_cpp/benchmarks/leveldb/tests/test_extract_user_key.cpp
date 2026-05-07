#include "../src/leveldb_functions.h"
#include <cassert>
#include <cstring>

int main() {
    // Test 1: 5-byte user key + 8-byte tag
    char buf1[13];
    memcpy(buf1, "hello", 5);
    memset(buf1 + 5, 0xAA, 8);
    leveldb::Slice ik1(buf1, 13);
    leveldb::Slice uk1 = leveldb::ExtractUserKey(ik1);
    assert(uk1.size() == 5);
    assert(memcmp(uk1.data(), "hello", 5) == 0);
    assert(uk1.data() == buf1);  // must point into original buffer

    // Test 2: Minimum — 8-byte key means 0-byte user key
    char buf2[8];
    memset(buf2, 0xBB, 8);
    leveldb::Slice ik2(buf2, 8);
    leveldb::Slice uk2 = leveldb::ExtractUserKey(ik2);
    assert(uk2.size() == 0);

    // Test 3: 12-byte user key
    char buf3[20];
    memcpy(buf3, "longeruserkey", 12);
    memset(buf3 + 12, 0xCC, 8);
    leveldb::Slice ik3(buf3, 20);
    leveldb::Slice uk3 = leveldb::ExtractUserKey(ik3);
    assert(uk3.size() == 12);
    assert(memcmp(uk3.data(), "longeruserkey", 12) == 0);

    // Test 4: Tag bytes are NOT included in user key
    char buf4[10];
    memcpy(buf4, "ab", 2);
    memset(buf4 + 2, 0xFF, 8);
    leveldb::Slice ik4(buf4, 10);
    leveldb::Slice uk4 = leveldb::ExtractUserKey(ik4);
    assert(uk4.size() == 2);
    assert(static_cast<uint8_t>(uk4.data()[0]) == 'a');
    assert(static_cast<uint8_t>(uk4.data()[1]) == 'b');

    return 0;
}
