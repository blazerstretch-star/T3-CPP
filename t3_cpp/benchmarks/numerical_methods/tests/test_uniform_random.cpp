#include "../src/numerical_functions.h"
#include <cassert>
#include <iostream>

int main() {
    using namespace NumericalMethods;
    
    unsigned int seed = 12345;
    unsigned int a = 1664525;
    unsigned int c = 1013904223;
    unsigned int m = 2147483648;
    
    // ========================================
    // PROPERTY 1: Range validation [min, max]
    // ========================================
    for (int i = 0; i < 1000; i++) {
        double r = uniform_random(5.0, 10.0, seed, a, c, m);
        assert(r >= 5.0 && r <= 10.0 && "Value must be in [5.0, 10.0]");
    }
    
    unsigned int seed2 = 54321;
    for (int i = 0; i < 1000; i++) {
        double r = uniform_random(0.0, 1.0, seed2, a, c, m);
        assert(r >= 0.0 && r <= 1.0 && "Value must be in [0.0, 1.0]");
    }
    
    unsigned int seed3 = 98765;
    for (int i = 0; i < 1000; i++) {
        double r = uniform_random(-5.0, 5.0, seed3, a, c, m);
        assert(r >= -5.0 && r <= 5.0 && "Value must be in [-5.0, 5.0]");
    }
    
    // ========================================
    // PROPERTY 2: Negative ranges work correctly
    // ========================================
    unsigned int seed4 = 11111;
    for (int i = 0; i < 100; i++) {
        double r = uniform_random(-10.0, -5.0, seed4, a, c, m);
        assert(r >= -10.0 && r <= -5.0 && "Negative range should work");
    }
    
    // ========================================
    // PROPERTY 3: Large ranges work correctly
    // ========================================
    unsigned int seed5 = 22222;
    for (int i = 0; i < 100; i++) {
        double r = uniform_random(0.0, 1000.0, seed5, a, c, m);
        assert(r >= 0.0 && r <= 1000.0 && "Large range should work");
    }
    
    // ========================================
    // PROPERTY 4: Deterministic behavior
    // ========================================
    unsigned int seed6a = 33333;
    unsigned int seed6b = 33333;
    double r1 = uniform_random(0.0, 100.0, seed6a, a, c, m);
    double r2 = uniform_random(0.0, 100.0, seed6b, a, c, m);
    assert(r1 == r2 && "Same seed should produce same value");
    
    // ========================================
    // PROPERTY 5: Coverage of range (statistical)
    // ========================================
    unsigned int seed7 = 44444;
    int low_count = 0, high_count = 0;
    for (int i = 0; i < 1000; i++) {
        double r = uniform_random(0.0, 10.0, seed7, a, c, m);
        if (r < 5.0) low_count++;
        else high_count++;
    }
    // Should be roughly 50/50 (allow 30% deviation)
    assert(low_count > 350 && low_count < 650 && "Should cover lower half");
    assert(high_count > 350 && high_count < 650 && "Should cover upper half");
    
    // ========================================
    // PROPERTY 6: Scaling property
    // ========================================
    // uniform_random(a, b) should scale lcg_rand output
    unsigned int seed8a = 55555;
    unsigned int seed8b = 55555;
    double lcg_val = lcg_rand(seed8a, a, c, m);
    double uniform_val = uniform_random(10.0, 20.0, seed8b, a, c, m);
    // uniform should be: 10 + (20-10) * lcg_val = 10 + 10*lcg_val
    assert(std::abs(uniform_val - (10.0 + 10.0 * lcg_val)) < 1e-10 && "Scaling should be correct");
    
    // ========================================
    // PROPERTY 7: Zero-width range
    // ========================================
    unsigned int seed9 = 66666;
    double r_same = uniform_random(5.0, 5.0, seed9, a, c, m);
    assert(std::abs(r_same - 5.0) < 1e-10 && "Zero-width range should return the value");
    
    // ========================================
    // PROPERTY 8: Different values in sequence
    // ========================================
    unsigned int seed10 = 77777;
    double prev = uniform_random(0.0, 100.0, seed10, a, c, m);
    int different_count = 0;
    for (int i = 0; i < 100; i++) {
        double curr = uniform_random(0.0, 100.0, seed10, a, c, m);
        if (std::abs(curr - prev) > 1e-10) different_count++;
        prev = curr;
    }
    assert(different_count > 90 && "Sequence should have mostly different values");
    
    std::cout << "test_uniform_random PASSED" << std::endl;
    return 0;
}
