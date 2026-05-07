#include "../src/leveldb_functions.h"
#include <cassert>
#include <cstring>
#include <string>

int main() {
    // Test 1: Basic decode, data content verified
    std::string s1;
    leveldb::PutLengthPrefixedSlice(&s1, leveldb::Slice("hello", 5));
    leveldb::Slice in1(s1);
    leveldb::Slice r1;
    assert(leveldb::GetLengthPrefixedSlice(&in1, &r1));
    assert(r1.size() == 5);
    assert(memcmp(r1.data(), "hello", 5) == 0);
    assert(in1.size() == 0);

    // Test 2: Empty slice value
    std::string s2;
    leveldb::PutLengthPrefixedSlice(&s2, leveldb::Slice("", 0));
    leveldb::Slice in2(s2);
    leveldb::Slice r2;
    assert(leveldb::GetLengthPrefixedSlice(&in2, &r2));
    assert(r2.size() == 0);
    assert(in2.size() == 0);

    // Test 3: Sequential decode, slice advances between entries
    std::string s3;
    leveldb::PutLengthPrefixedSlice(&s3, leveldb::Slice("abc", 3));
    leveldb::PutLengthPrefixedSlice(&s3, leveldb::Slice("defgh", 5));
    leveldb::Slice in3(s3);
    leveldb::Slice ra, rb;
    assert(leveldb::GetLengthPrefixedSlice(&in3, &ra));
    assert(ra.size() == 3 && memcmp(ra.data(), "abc", 3) == 0);
    assert(leveldb::GetLengthPrefixedSlice(&in3, &rb));
    assert(rb.size() == 5 && memcmp(rb.data(), "defgh", 5) == 0);
    assert(in3.size() == 0);

    // Test 4: Truncated data (length prefix says 10 but only 3 bytes follow) → false
    std::string s4;
    leveldb::PutVarint32(&s4, 10);
    s4.append("abc");  // only 3 bytes, not 10
    leveldb::Slice in4(s4);
    leveldb::Slice r4;
    assert(!leveldb::GetLengthPrefixedSlice(&in4, &r4));

    // Test 5: Empty input → false
    leveldb::Slice in5("", 0);
    leveldb::Slice r5;
    assert(!leveldb::GetLengthPrefixedSlice(&in5, &r5));

    // Test 6: Result points into original buffer (no copy)
    std::string s6;
    leveldb::PutLengthPrefixedSlice(&s6, leveldb::Slice("xyz", 3));
    leveldb::Slice in6(s6);
    leveldb::Slice r6;
    assert(leveldb::GetLengthPrefixedSlice(&in6, &r6));
    assert(r6.data() >= s6.data() && r6.data() < s6.data() + s6.size());

    return 0;
}
