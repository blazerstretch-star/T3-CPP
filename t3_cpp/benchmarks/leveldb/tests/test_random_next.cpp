#include "../src/leveldb_functions.h"
#include <cassert>
#include <cstdint>
#include <set>

int main() {
    static const uint32_t M = 2147483647u;  // 2^31 - 1
    static const uint64_t A = 16807u;

    // --- All 1000 values in (0, M) ---
    leveldb::Random r1(1);
    for (int i = 0; i < 1000; i++) {
        uint32_t v = r1.Next();
        assert(v > 0 && v < M);
    }

    // --- Pinned: first two values for seed 1 ---
    leveldb::Random r2(1);
    assert(r2.Next() == 16807u);
    assert(r2.Next() == 282475249u);

    // --- Verify full LCG formula for 20 steps from seed 1 ---
    leveldb::Random r3(1);
    uint32_t seed = 1;
    for (int i = 0; i < 20; i++) {
        uint64_t product = static_cast<uint64_t>(seed) * A;
        seed = static_cast<uint32_t>((product >> 31) + (product & M));
        if (seed > M) seed -= M;
        assert(r3.Next() == seed);
    }

    // --- Same seed → same sequence ---
    leveldb::Random r4(999), r5(999);
    for (int i = 0; i < 100; i++) assert(r4.Next() == r5.Next());

    // --- 100 values from seed 42 are all distinct ---
    leveldb::Random r6(42);
    std::set<uint32_t> seen;
    for (int i = 0; i < 100; i++) seen.insert(r6.Next());
    assert(seen.size() == 100);

    return 0;
}
