#include "../src/leveldb_functions.h"
#include <cassert>
#include <cstring>
#include <cstdint>
#include <vector>
#include <string>

int main() {
    using namespace leveldb;

    // --- Test 1: First entry — exact varint header: shared=0, non_shared=4, vlen=4 ---
    std::string buf, last_key;
    std::vector<uint32_t> restarts = {0};
    int counter = 0;

    BlockBuilder_Add(&buf, &last_key, Slice("key1", 4), Slice("val1", 4), 16, &counter, &restarts);
    assert(counter == 1);
    assert(last_key == "key1");

    Slice in(buf);
    uint32_t shared, non_shared, vlen;
    assert(GetVarint32(&in, &shared));     assert(shared == 0);
    assert(GetVarint32(&in, &non_shared)); assert(non_shared == 4);
    assert(GetVarint32(&in, &vlen));       assert(vlen == 4);
    assert(memcmp(in.data(), "key1", 4) == 0);
    assert(memcmp(in.data() + 4, "val1", 4) == 0);

    // --- Test 2: Second entry — exact prefix compression: shared=3 ("key"), non_shared=1 ("2") ---
    size_t buf_before = buf.size();
    BlockBuilder_Add(&buf, &last_key, Slice("key2", 4), Slice("val2", 4), 16, &counter, &restarts);
    assert(counter == 2);
    assert(last_key == "key2");

    Slice in2(buf.data() + buf_before, buf.size() - buf_before);
    uint32_t sh2, nsh2, vl2;
    assert(GetVarint32(&in2, &sh2));  assert(sh2 == 3);   // "key" shared
    assert(GetVarint32(&in2, &nsh2)); assert(nsh2 == 1);  // "2" non-shared
    assert(GetVarint32(&in2, &vl2));  assert(vl2 == 4);
    assert(memcmp(in2.data(), "2", 1) == 0);              // only delta stored
    assert(memcmp(in2.data() + 1, "val2", 4) == 0);

    // --- Test 3: Restart point at exact buffer offset ---
    std::string buf2, last_key2;
    std::vector<uint32_t> restarts2 = {0};
    int counter2 = 0;
    BlockBuilder_Add(&buf2, &last_key2, Slice("aaa", 3), Slice("v1", 2), 2, &counter2, &restarts2);
    BlockBuilder_Add(&buf2, &last_key2, Slice("aab", 3), Slice("v2", 2), 2, &counter2, &restarts2);
    size_t offset_before = buf2.size();
    BlockBuilder_Add(&buf2, &last_key2, Slice("bbb", 3), Slice("v3", 2), 2, &counter2, &restarts2);
    assert(restarts2.size() == 2);
    assert(restarts2[1] == offset_before);
    assert(counter2 == 1);

    // --- Test 4: At restart point, shared=0 (no prefix compression) ---
    Slice in3(buf2.data() + offset_before, buf2.size() - offset_before);
    uint32_t sh3, nsh3, vl3;
    assert(GetVarint32(&in3, &sh3));  assert(sh3 == 0);   // restart: no sharing
    assert(GetVarint32(&in3, &nsh3)); assert(nsh3 == 3);  // full key "bbb"
    assert(GetVarint32(&in3, &vl3));  assert(vl3 == 2);
    assert(memcmp(in3.data(), "bbb", 3) == 0);

    return 0;
}
