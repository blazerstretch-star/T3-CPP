#include "../src/lfs_functions.h"
#include <cassert>
#include <iostream>

int main() {
    // ========================================
    // PROPERTY 1: Result is multiple of alignment
    // ========================================
    auto check_multiple = [](lfs_size_t a, lfs_size_t alignment) {
        if (alignment == 0) return; // Skip division by zero
        lfs_size_t result = lfs_aligndown(a, alignment);
        assert(result % alignment == 0 && 
               "Property: aligndown result must be multiple of alignment");
    };
    
    // ========================================
    // PROPERTY 2: Result <= input
    // ========================================
    auto check_less_or_equal = [](lfs_size_t a, lfs_size_t alignment) {
        if (alignment == 0) return;
        lfs_size_t result = lfs_aligndown(a, alignment);
        assert(result <= a && 
               "Property: aligndown(a) must be <= a");
    };
    
    // ========================================
    // PROPERTY 3: Distance from input < alignment
    // ========================================
    auto check_distance = [](lfs_size_t a, lfs_size_t alignment) {
        if (alignment == 0) return;
        lfs_size_t result = lfs_aligndown(a, alignment);
        assert((a - result) < alignment && 
               "Property: distance from input must be < alignment");
    };
    
    // ========================================
    // PROPERTY 4: Idempotence - aligndown(aligndown(a)) == aligndown(a)
    // ========================================
    auto check_idempotence = [](lfs_size_t a, lfs_size_t alignment) {
        if (alignment == 0) return;
        lfs_size_t result1 = lfs_aligndown(a, alignment);
        lfs_size_t result2 = lfs_aligndown(result1, alignment);
        assert(result1 == result2 && 
               "Property: aligndown must be idempotent");
    };
    
    // ========================================
    // PROPERTY 5: Already aligned values unchanged
    // ========================================
    auto check_aligned_unchanged = [](lfs_size_t multiple, lfs_size_t alignment) {
        if (alignment == 0) return;
        lfs_size_t aligned = multiple * alignment;
        assert(lfs_aligndown(aligned, alignment) == aligned && 
               "Property: already aligned values must be unchanged");
    };
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: Basic alignment
    {
        assert(lfs_aligndown(10, 4) == 8);
        assert(lfs_aligndown(11, 4) == 8);
        assert(lfs_aligndown(12, 4) == 12);
        check_multiple(10, 4);
        check_less_or_equal(10, 4);
        check_distance(10, 4);
    }
    
    // Test 2: Already aligned values
    {
        assert(lfs_aligndown(16, 4) == 16);
        assert(lfs_aligndown(16, 8) == 16);
        assert(lfs_aligndown(16, 16) == 16);
        check_aligned_unchanged(4, 4);
        check_aligned_unchanged(8, 4);
        check_idempotence(16, 4);
    }
    
    // Test 3: Zero value
    {
        assert(lfs_aligndown(0, 4) == 0);
        assert(lfs_aligndown(0, 8) == 0);
        assert(lfs_aligndown(0, 16) == 0);
        check_multiple(0, 4);
        check_idempotence(0, 4);
    }
    
    // Test 4: Alignment of 1 (no change)
    {
        assert(lfs_aligndown(10, 1) == 10);
        assert(lfs_aligndown(99, 1) == 99);
        assert(lfs_aligndown(1000, 1) == 1000);
        check_idempotence(10, 1);
    }
    
    // Test 5: Power of 2 alignments (common in filesystems)
    {
        assert(lfs_aligndown(100, 16) == 96);
        assert(lfs_aligndown(100, 32) == 96);
        assert(lfs_aligndown(100, 64) == 64);
        assert(lfs_aligndown(1000, 256) == 768);
        check_multiple(100, 16);
        check_distance(100, 16);
    }
    
    // Test 6: Large values
    {
        assert(lfs_aligndown(4095, 4096) == 0);
        assert(lfs_aligndown(4096, 4096) == 4096);
        assert(lfs_aligndown(8191, 4096) == 4096);
        check_multiple(4095, 4096);
        check_less_or_equal(4095, 4096);
    }
    
    // Test 7: Non-power-of-2 alignments
    {
        assert(lfs_aligndown(100, 3) == 99);
        assert(lfs_aligndown(100, 7) == 98);
        assert(lfs_aligndown(100, 11) == 99);
        check_multiple(100, 3);
        check_distance(100, 3);
    }
    
    // Test 8: Boundary cases
    {
        assert(lfs_aligndown(1, 2) == 0);
        assert(lfs_aligndown(2, 2) == 2);
        assert(lfs_aligndown(3, 2) == 2);
        check_idempotence(1, 2);
        check_idempotence(2, 2);
    }
    
    // Test 9: Stress test - verify properties
    {
        lfs_size_t alignments[] = {1, 2, 4, 8, 16, 32, 64, 128, 256};
        for (auto align : alignments) {
            for (lfs_size_t val = 0; val < 1000; val += 13) {
                check_multiple(val, align);
                check_less_or_equal(val, align);
                check_distance(val, align);
                check_idempotence(val, align);
            }
        }
    }
    
    std::cout << "All lfs_aligndown tests passed!" << std::endl;
    return 0;
}
