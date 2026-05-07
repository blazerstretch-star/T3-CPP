#include "../src/jemalloc_utils.h"
#include <cassert>
#include <set>

using namespace JemallocUtils;

int main() {
    // PROPERTY 1: Generates different values in sequence
    uint64_t state = 12345;
    uint64_t val1 = ckh_prng(&state);
    uint64_t val2 = ckh_prng(&state);
    assert(val1 != val2 && "Consecutive calls must produce different values");
    
    // PROPERTY 2: State is modified after each call
    uint64_t state2 = 12345;
    uint64_t original_state = state2;
    ckh_prng(&state2);
    assert(state2 != original_state && "State must be modified by PRNG");
    
    // PROPERTY 3: Determinism - same initial state produces same sequence
    uint64_t state3 = 12345;
    uint64_t state4 = 12345;
    uint64_t seq1[10], seq2[10];
    
    for (int i = 0; i < 10; i++) {
        seq1[i] = ckh_prng(&state3);
        seq2[i] = ckh_prng(&state4);
    }
    
    for (int i = 0; i < 10; i++) {
        assert(seq1[i] == seq2[i] && "Same seed must produce same sequence");
    }
    
    // PROPERTY 4: Different seeds produce different sequences
    uint64_t state5 = 12345;
    uint64_t state6 = 54321;
    uint64_t val5 = ckh_prng(&state5);
    uint64_t val6 = ckh_prng(&state6);
    assert(val5 != val6 && "Different seeds should produce different values");
    
    // PROPERTY 5: Return value matches modified state
    uint64_t state7 = 12345;
    uint64_t returned = ckh_prng(&state7);
    assert(returned == state7 && "Return value should match new state");
    
    // PROPERTY 6: Generates non-zero values (for most seeds)
    uint64_t state8 = 12345;
    bool has_nonzero = false;
    for (int i = 0; i < 10; i++) {
        if (ckh_prng(&state8) != 0) {
            has_nonzero = true;
            break;
        }
    }
    assert(has_nonzero && "Should generate non-zero values");
    
    // PROPERTY 7: Long sequence has no immediate repeats
    uint64_t state9 = 12345;
    std::set<uint64_t> seen;
    bool has_unique = true;
    for (int i = 0; i < 100; i++) {
        uint64_t val = ckh_prng(&state9);
        if (seen.count(val)) {
            has_unique = false;
            break;
        }
        seen.insert(val);
    }
    assert(has_unique && "Should not repeat values in short sequence");
    
    // PROPERTY 8: Zero state handling
    uint64_t state10 = 0;
    uint64_t val_from_zero = ckh_prng(&state10);
    assert(state10 != 0 && "State should change even from zero");
    
    return 0;
}
