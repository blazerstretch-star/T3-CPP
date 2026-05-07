#include "../src/leveldb_functions.h"
#include <cassert>
#include <cstdint>

int main() {
    // --- Property 1: OneIn(1) always returns true ---
    leveldb::Random r1(300);
    for (int i = 0; i < 50; i++) assert(r1.OneIn(1) == true);

    // --- Property 2: Pinned values — OneIn(n) == (Next() % n == 0) ---
    // seed=1: Next()=16807, 16807 % 2 = 1 → false
    leveldb::Random r2(1);
    assert(r2.OneIn(2) == false);
    // Next()=282475249, 282475249 % 2 = 1 → false
    assert(r2.OneIn(2) == false);
    // Next()=1622650073, 1622650073 % 2 = 1 → false
    assert(r2.OneIn(2) == false);

    // --- Property 3: OneIn(2) produces both true and false ---
    // Use 200 trials — P(never true) = 0.5^200 ≈ 0, safe
    leveldb::Random r3(200);
    bool found_true = false, found_false = false;
    for (int i = 0; i < 200; i++) {
        bool v = r3.OneIn(2);
        if (v) found_true = true;
        else found_false = true;
    }
    assert(found_true && found_false);

    // --- Property 4: Frequency ordering — OneIn(2) fires more than OneIn(4) ---
    // This is implementation-agnostic: any correct impl must satisfy this.
    leveldb::Random r4a(42), r4b(42);
    int count2 = 0, count4 = 0;
    for (int i = 0; i < 2000; i++) {
        if (r4a.OneIn(2)) count2++;
        if (r4b.OneIn(4)) count4++;
    }
    assert(count2 > count4);  // P(1/2) > P(1/4) always

    // --- Property 5: OneIn(n) frequency is in a generous range ---
    // For n=4 over 2000 trials: expected=500, 6-sigma=[227,773]
    // Use [100, 900] — accepts any correct impl
    leveldb::Random r5(42);
    int count = 0;
    for (int i = 0; i < 2000; i++) if (r5.OneIn(4)) count++;
    assert(count >= 100 && count <= 900);

    // --- Property 6: OneIn(large_n) fires rarely ---
    // For n=100 over 10000 trials: expected=100, allow [10, 400]
    leveldb::Random r6(99);
    int rare = 0;
    for (int i = 0; i < 10000; i++) if (r6.OneIn(100)) rare++;
    assert(rare >= 10 && rare <= 400);

    // --- Property 7: Deterministic — same seed gives same sequence ---
    leveldb::Random ra(77), rb(77);
    for (int i = 0; i < 100; i++)
        assert(ra.OneIn(3) == rb.OneIn(3));

    return 0;
}
