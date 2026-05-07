#include "../src/leveldb_functions.h"
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

int main() {
    using namespace leveldb;

    // --- Exact size: n=3, bpk=10 → bits=max(30,64)=64 → bytes=8 → total=9 ---
    Slice keys1[] = {Slice("key1"), Slice("key2"), Slice("key3")};
    std::string f1;
    BloomFilter_CreateFilter(keys1, 3, 10, &f1);
    assert(f1.size() == 9);

    // --- Last byte = k = floor(10 * 0.69) = 6 ---
    assert(static_cast<uint8_t>(f1.back()) == 6);

    // --- n=1, bpk=1 → k=1, size=9 ---
    Slice keys2[] = {Slice("x")};
    std::string f2;
    BloomFilter_CreateFilter(keys2, 1, 1, &f2);
    assert(f2.size() == 9);
    assert(static_cast<uint8_t>(f2.back()) == 1);

    // --- Appends to existing content ---
    std::string f3 = "PREFIX";
    BloomFilter_CreateFilter(keys1, 3, 10, &f3);
    assert(f3.size() == 6 + 9);
    assert(f3.substr(0, 6) == "PREFIX");

    // --- bpk=20 → k=13 ---
    std::string f4;
    BloomFilter_CreateFilter(keys1, 3, 20, &f4);
    assert(static_cast<uint8_t>(f4.back()) == 13);

    // --- Fuzz: 50 random keys, no false negatives ---
    uint32_t lcg = 0xDEADBEEFu;
    std::vector<std::string> key_strs(50);
    std::vector<Slice> key_slices(50);
    for (int i = 0; i < 50; i++) {
        lcg = lcg * 1664525u + 1013904223u;
        key_strs[i] = "fuzz_key_" + std::to_string(lcg);
        key_slices[i] = Slice(key_strs[i]);
    }
    std::string fuzz_filter;
    BloomFilter_CreateFilter(key_slices.data(), 50, 10, &fuzz_filter);

    // Exact size: n=50, bpk=10 → bits=500 → bytes=63 → total=64
    assert(fuzz_filter.size() == 64);
    assert(static_cast<uint8_t>(fuzz_filter.back()) == 6);

    // All inserted keys must match (no false negatives)
    for (int i = 0; i < 50; i++)
        assert(BloomFilter_KeyMayMatch(key_slices[i], Slice(fuzz_filter)));

    return 0;
}
