#include "../src/leveldb_functions.h"
#include <cassert>
#include <cstring>
#include <cstdint>
#include <vector>
#include <string>

int main() {
    using namespace leveldb;

    // Test 1: Finish appends restart array + count in fixed32 little-endian
    std::string buf = "DATABLOCK";  // 9 bytes of fake block data
    std::vector<uint32_t> restarts = {0, 5, 8};

    size_t data_size = buf.size();
    Slice result = BlockBuilder_Finish(&buf, restarts);

    // Total size = data + 3 restarts * 4 bytes + 1 count * 4 bytes
    assert(buf.size() == data_size + 3 * 4 + 4);
    assert(result.size() == buf.size());
    assert(result.data() == buf.data());

    // Verify restart offsets encoded as little-endian uint32
    const char* trailer = buf.data() + data_size;
    assert(DecodeFixed32(trailer + 0) == 0u);
    assert(DecodeFixed32(trailer + 4) == 5u);
    assert(DecodeFixed32(trailer + 8) == 8u);
    // Verify count
    assert(DecodeFixed32(trailer + 12) == 3u);

    // Test 2: Single restart point {0}
    std::string buf2;
    std::vector<uint32_t> restarts2 = {0};
    Slice r2 = BlockBuilder_Finish(&buf2, restarts2);
    assert(buf2.size() == 8);  // 1 restart * 4 + count * 4
    assert(DecodeFixed32(buf2.data()) == 0u);
    assert(DecodeFixed32(buf2.data() + 4) == 1u);

    // Test 3: Empty restarts list
    std::string buf3;
    std::vector<uint32_t> restarts3;
    Slice r3 = BlockBuilder_Finish(&buf3, restarts3);
    assert(buf3.size() == 4);  // only count=0
    assert(DecodeFixed32(buf3.data()) == 0u);

    // Test 4: Returned Slice covers entire buffer including trailer
    std::string buf4 = "ABC";
    std::vector<uint32_t> restarts4 = {0, 3};
    Slice r4 = BlockBuilder_Finish(&buf4, restarts4);
    assert(r4.size() == buf4.size());
    assert(r4.data() == buf4.data());

    return 0;
}
