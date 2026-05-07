#include "../src/leveldb_functions.h"
#include <cassert>
#include <cstring>
#include <cstdint>
#include <string>

int main() {
    // Test 1: Full round-trip — verify all three output fields
    std::string buf;
    leveldb::AppendInternalKey(&buf, leveldb::Slice("mykey", 5), 100, 1);
    leveldb::Slice uk; uint64_t seq; uint8_t type;
    assert(leveldb::ParseInternalKey(leveldb::Slice(buf), &uk, &seq, &type));
    assert(uk.size() == 5);
    assert(memcmp(uk.data(), "mykey", 5) == 0);
    assert(seq == 100);
    assert(type == 1);

    // Test 2: type=0 (deletion) is valid
    std::string buf2;
    leveldb::AppendInternalKey(&buf2, leveldb::Slice("k", 1), 1, 0);
    leveldb::Slice uk2; uint64_t seq2; uint8_t type2;
    assert(leveldb::ParseInternalKey(leveldb::Slice(buf2), &uk2, &seq2, &type2));
    assert(seq2 == 1);
    assert(type2 == 0);

    // Test 3: type > 1 → returns false (invalid value type per source)
    std::string buf3;
    leveldb::AppendInternalKey(&buf3, leveldb::Slice("k", 1), 1, 2);
    leveldb::Slice uk3; uint64_t seq3; uint8_t type3;
    assert(!leveldb::ParseInternalKey(leveldb::Slice(buf3), &uk3, &seq3, &type3));

    // Test 4: Key shorter than 8 bytes → false
    leveldb::Slice short_key("abc", 3);
    leveldb::Slice uk4; uint64_t seq4; uint8_t type4;
    assert(!leveldb::ParseInternalKey(short_key, &uk4, &seq4, &type4));

    // Test 5: Exactly 8 bytes → 0-byte user key, valid
    std::string buf5;
    leveldb::AppendInternalKey(&buf5, leveldb::Slice("", 0), 42, 1);
    assert(buf5.size() == 8);
    leveldb::Slice uk5; uint64_t seq5; uint8_t type5;
    assert(leveldb::ParseInternalKey(leveldb::Slice(buf5), &uk5, &seq5, &type5));
    assert(uk5.size() == 0);
    assert(seq5 == 42);
    assert(type5 == 1);

    // Test 6: User key data pointer points into original buffer
    std::string buf6;
    leveldb::AppendInternalKey(&buf6, leveldb::Slice("hello", 5), 7, 1);
    leveldb::Slice uk6; uint64_t seq6; uint8_t type6;
    assert(leveldb::ParseInternalKey(leveldb::Slice(buf6), &uk6, &seq6, &type6));
    assert(uk6.data() == buf6.data());

    return 0;
}
