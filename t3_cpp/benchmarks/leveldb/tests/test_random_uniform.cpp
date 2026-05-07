#include "../src/leveldb_functions.h"
#include <cassert>
#include <cstdint>

int main() {
    // --- Property 1: Uniform(n) always in [0, n) ---
    leveldb::Random r1(100);
    for (int i = 0; i < 500; i++) {
        uint32_t u = r1.Uniform(10);
        assert(u < 10u);
    }

    // --- Property 2: Uniform(1) always returns 0 ---
    leveldb::Random r2(200);
    for (int i = 0; i < 50; i++) assert(r2.Uniform(1) == 0);

    // --- Property 3: Pinned value — Uniform(n) == Next() % n ---
    // seed=1: Next()=16807, Uniform(100) = 16807 % 100 = 7
    leveldb::Random r3(1);
    assert(r3.Uniform(100) == 7u);
    // Next()=282475249, Uniform(100) = 282475249 % 100 = 49
    assert(r3.Uniform(100) == 49u);

    // --- Property 4: Range check for many different n values ---
    leveldb::Random r4(42);
    for (int n : {2, 3, 5, 7, 10, 100, 1000, 1000000}) {
        for (int i = 0; i < 50; i++) {
            uint32_t u = r4.Uniform(n);
            assert(u < (uint32_t)n);
        }
    }

    // --- Property 5: Uniform(2) produces both 0 and 1 ---
    // Use 500 trials — P(never see 0) = 0.5^500 ≈ 0, completely safe
    leveldb::Random r5(77);
    bool saw0 = false, saw1 = false;
    for (int i = 0; i < 500; i++) {
        uint32_t u = r5.Uniform(2);
        assert(u < 2u);
        if (u == 0) saw0 = true;
        if (u == 1) saw1 = true;
    }
    assert(saw0 && saw1);

    // --- Property 6: All values in [0, n) are reachable for small n ---
    // For n=4 over 1000 trials, all 4 values must appear.
    leveldb::Random r6(55);
    bool seen[4] = {};
    for (int i = 0; i < 1000; i++) seen[r6.Uniform(4)] = true;
    for (int i = 0; i < 4; i++) assert(seen[i]);

    // --- Property 7: Frequency ordering — Uniform(2) hits each value ~50% ---
    // Allow generous range [200, 800] out of 1000 trials
    leveldb::Random r7(33);
    int count0 = 0;
    for (int i = 0; i < 1000; i++) if (r7.Uniform(2) == 0) count0++;
    assert(count0 >= 200 && count0 <= 800);

    // --- Property 8: Deterministic — same seed gives same sequence ---
    leveldb::Random ra(99), rb(99);
    for (int i = 0; i < 100; i++)
        assert(ra.Uniform(1000) == rb.Uniform(1000));

    return 0;
}
