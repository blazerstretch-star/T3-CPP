#include "../src/numerical_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    using namespace NumericalMethods;
    
    // ========================================
    // PROPERTY 1: Range validation [0, 1]
    // ========================================
    unsigned int seed = 12345;
    unsigned int a = 1664525;
    unsigned int c = 1013904223;
    unsigned int m = 2147483648;
    
    for (int i = 0; i < 1000; i++) {
        double r = lcg_rand(seed, a, c, m);
        assert(r >= 0.0 && r <= 1.0 && "LCG output must be in [0, 1]");
    }
    
    // ========================================
    // PROPERTY 2: Deterministic - same seed produces same sequence
    // ========================================
    unsigned int seed1 = 12345;
    unsigned int seed2 = 12345;
    
    double r1 = lcg_rand(seed1, a, c, m);
    double r2 = lcg_rand(seed2, a, c, m);
    assert(r1 == r2 && "Same initial seed should produce same first value");
    
    double r3 = lcg_rand(seed1, a, c, m);
    double r4 = lcg_rand(seed2, a, c, m);
    assert(r3 == r4 && "Same seed should produce same second value");
    
    // ========================================
    // PROPERTY 3: Different values in sequence
    // ========================================
    unsigned int seed3 = 54321;
    double prev = lcg_rand(seed3, a, c, m);
    int different_count = 0;
    for (int i = 0; i < 100; i++) {
        double curr = lcg_rand(seed3, a, c, m);
        if (curr != prev) different_count++;
        prev = curr;
    }
    assert(different_count > 90 && "Sequence should have mostly different consecutive values");
    
    // ========================================
    // PROPERTY 4: Seed modification (seed is updated by reference)
    // ========================================
    unsigned int seed4 = 99999;
    unsigned int original_seed = seed4;
    lcg_rand(seed4, a, c, m);
    assert(seed4 != original_seed && "Seed should be modified after call");
    
    // ========================================
    // PROPERTY 5: Different seeds produce different sequences
    // ========================================
    unsigned int seed5 = 11111;
    unsigned int seed6 = 22222;
    double r5 = lcg_rand(seed5, a, c, m);
    double r6 = lcg_rand(seed6, a, c, m);
    assert(r5 != r6 && "Different seeds should produce different values");
    
    // ========================================
    // PROPERTY 6: Full range coverage (statistical)
    // ========================================
    unsigned int seed7 = 77777;
    int low_count = 0, mid_count = 0, high_count = 0;
    for (int i = 0; i < 1000; i++) {
        double r = lcg_rand(seed7, a, c, m);
        if (r < 0.33) low_count++;
        else if (r < 0.67) mid_count++;
        else high_count++;
    }
    // Each third should have roughly 333 values (allow 20% deviation)
    assert(low_count > 200 && low_count < 500 && "Should cover low range");
    assert(mid_count > 200 && mid_count < 500 && "Should cover mid range");
    assert(high_count > 200 && high_count < 500 && "Should cover high range");
    
    // ========================================
    // PROPERTY 7: Non-zero values (should not always return 0)
    // ========================================
    unsigned int seed8 = 88888;
    int nonzero_count = 0;
    for (int i = 0; i < 100; i++) {
        double r = lcg_rand(seed8, a, c, m);
        if (r > 0.0) nonzero_count++;
    }
    assert(nonzero_count > 95 && "Most values should be non-zero");
    
    std::cout << "test_lcg_rand PASSED" << std::endl;
    return 0;
}
