#include "../src/leveldb_functions.h"
#include <cassert>
#include <cstdint>
#include <string>

int main() {
    using namespace leveldb;

    // --- Property 1: No false negatives — inserted keys MUST always match ---
    // This is a hard guarantee of any correct bloom filter implementation.
    Slice keys[] = {Slice("key1"), Slice("key2"), Slice("key3"), Slice("key4")};
    std::string filter;
    BloomFilter_CreateFilter(keys, 4, 10, &filter);
    for (auto& k : keys)
        assert(BloomFilter_KeyMayMatch(k, Slice(filter)));

    // --- Property 2: Empty/tiny filter → false (not enough data to match) ---
    assert(!BloomFilter_KeyMayMatch(Slice("key1"), Slice("", 0)));
    assert(!BloomFilter_KeyMayMatch(Slice("key1"), Slice("x", 1)));

    // --- Property 3: k > 30 in last byte → always true (reserved encoding) ---
    char big_k[3] = {0x00, 0x00, 31};
    assert(BloomFilter_KeyMayMatch(Slice("anything"), Slice(big_k, 3)));

    // --- Property 4: No false negatives for 100 keys ---
    std::string inserted_keys[100];
    Slice key_slices[100];
    for (int i = 0; i < 100; i++) {
        inserted_keys[i] = "inserted_key_" + std::to_string(i);
        key_slices[i] = Slice(inserted_keys[i]);
    }
    std::string big_filter;
    BloomFilter_CreateFilter(key_slices, 100, 10, &big_filter);
    for (int i = 0; i < 100; i++)
        assert(BloomFilter_KeyMayMatch(key_slices[i], Slice(big_filter)));

    // --- Property 5: False positive rate < 10% (generous bound) ---
    // Theoretical rate for bpk=10, k=6 is ~2%. Allow up to 10% for any
    // correct implementation regardless of hash function details.
    int false_positives = 0;
    for (int i = 0; i < 1000; i++) {
        std::string absent = "absent_key_" + std::to_string(i);
        if (BloomFilter_KeyMayMatch(Slice(absent), Slice(big_filter)))
            false_positives++;
    }
    assert(false_positives <= 100);  // < 10% — generous for any correct impl

    // --- Property 6: A key NOT in filter should not always match ---
    // (Verifies the filter is not trivially returning true for everything)
    int always_match = 0;
    for (int i = 0; i < 100; i++) {
        std::string absent = "definitely_not_inserted_" + std::to_string(i * 9999);
        if (BloomFilter_KeyMayMatch(Slice(absent), Slice(big_filter)))
            always_match++;
    }
    // If all 100 absent keys match, the filter is broken (trivially true)
    assert(always_match < 100);

    // --- Property 7: Larger bpk → fewer false positives (ordering property) ---
    // This is implementation-agnostic: more bits per key = better filter.
    std::string filter10, filter20;
    BloomFilter_CreateFilter(key_slices, 100, 10, &filter10);
    BloomFilter_CreateFilter(key_slices, 100, 20, &filter20);
    int fp10 = 0, fp20 = 0;
    for (int i = 0; i < 1000; i++) {
        std::string absent = "ordering_test_" + std::to_string(i);
        if (BloomFilter_KeyMayMatch(Slice(absent), Slice(filter10))) fp10++;
        if (BloomFilter_KeyMayMatch(Slice(absent), Slice(filter20))) fp20++;
    }
    // bpk=20 should have fewer or equal false positives than bpk=10
    assert(fp20 <= fp10 + 20);  // allow small variance

    return 0;
}
