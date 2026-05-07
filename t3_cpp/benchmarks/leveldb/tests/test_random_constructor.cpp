#include "../src/leveldb_functions.h"
#include <cassert>
#include <cstdint>

int main() {
    // Test 1: Same seed → same first Next() value (deterministic)
    leveldb::Random r1(12345), r2(12345);
    assert(r1.Next() == r2.Next());

    // Test 2: Different seeds → different first Next() values
    leveldb::Random r3(1), r4(2);
    assert(r3.Next() != r4.Next());

    // Test 3: Seed 0 is remapped to 1 — must not produce 0 from Next()
    // (LCG with seed 0 would stay at 0 forever)
    leveldb::Random r5(0);
    for (int i = 0; i < 10; i++) assert(r5.Next() > 0);

    // Test 4: Seed 2147483647 (M) is remapped to 1 — must not produce M forever
    leveldb::Random r6(2147483647u);
    for (int i = 0; i < 10; i++) {
        uint32_t v = r6.Next();
        assert(v > 0 && v < 2147483647u);
    }

    // Test 5: High bits of seed are masked (seed & 0x7FFFFFFF)
    // Seeds 0x80000001 and 0x00000001 both mask to 1 → same sequence
    leveldb::Random r7(0x80000001u), r8(0x00000001u);
    assert(r7.Next() == r8.Next());

    return 0;
}
