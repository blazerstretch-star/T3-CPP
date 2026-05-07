#include "../src/leveldb_functions.h"
#include <cassert>
#include <cstdint>

int main() {
    // --- Property 1: Height always in [1, max_height] for any max_height ---
    leveldb::Random r1(12345);
    for (int i = 0; i < 1000; i++) {
        int h = leveldb::SkipListRandomHeight(&r1, 12);
        assert(h >= 1 && h <= 12);
    }

    // --- Property 2: max_height=1 always returns 1 ---
    leveldb::Random r2(555);
    for (int i = 0; i < 200; i++)
        assert(leveldb::SkipListRandomHeight(&r2, 1) == 1);

    // --- Property 3: Height never exceeds max_height for various limits ---
    leveldb::Random r3(111);
    for (int max_h : {2, 3, 5, 7, 12}) {
        for (int i = 0; i < 200; i++)
            assert(leveldb::SkipListRandomHeight(&r3, max_h) <= max_h);
    }

    // --- Property 4: Exponential decay — count(h=1) > count(h=2) > count(h=3) ---
    // This holds for ANY branching factor >= 2, so it's implementation-agnostic.
    leveldb::Random r4(999);
    int counts[13] = {};
    for (int i = 0; i < 10000; i++)
        counts[leveldb::SkipListRandomHeight(&r4, 12)]++;
    assert(counts[1] > counts[2]);
    assert(counts[2] > counts[3]);
    assert(counts[3] > counts[4]);

    // --- Property 5: Height 1 is the majority (> 50% for any branching >= 2) ---
    // This is true for kBranching=2 (50%), 4 (75%), or any larger value.
    assert(counts[1] > 5000);  // > 50% of 10000

    // --- Property 6: High heights are rare (height >= 8 should be < 5% of 10000) ---
    int high_count = 0;
    for (int h = 8; h <= 12; h++) high_count += counts[h];
    assert(high_count < 500);  // < 5% — true for kBranching >= 2

    // --- Property 7: Deterministic — same seed gives same sequence ---
    leveldb::Random ra(42), rb(42);
    for (int i = 0; i < 100; i++)
        assert(leveldb::SkipListRandomHeight(&ra, 12) ==
               leveldb::SkipListRandomHeight(&rb, 12));

    // --- Property 8: Height 1 is achievable (not always > 1) ---
    leveldb::Random r5(77);
    bool saw_1 = false;
    for (int i = 0; i < 100; i++)
        if (leveldb::SkipListRandomHeight(&r5, 12) == 1) { saw_1 = true; break; }
    assert(saw_1);

    // --- Property 9: Heights > 1 are achievable (not always 1) ---
    leveldb::Random r6(88);
    bool saw_gt1 = false;
    for (int i = 0; i < 200; i++)
        if (leveldb::SkipListRandomHeight(&r6, 12) > 1) { saw_gt1 = true; break; }
    assert(saw_gt1);

    return 0;
}
