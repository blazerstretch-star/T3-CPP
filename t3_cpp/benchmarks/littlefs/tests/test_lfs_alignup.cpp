#include "../src/lfs_functions.h"
#include <cassert>
#include <iostream>

int main() {
    // ========================================
    // PROPERTY 1: Result is multiple of alignment
    // ========================================
    auto check_multiple = [](lfs_size_t a, lfs_size_t alignment) {
        if (alignment == 0) return;
        lfs_size_t result = lfs_alignup(a, alignment);
        assert(result % alignment == 0 && 
               "Property: alignup result must be multiple of alignment");
    };
    
    // ========================================
    // PROPERTY 2: Result >= input
    // ========================================
    auto check_greater_or_equal = [](lfs_size_t a, lfs_size_t alignment) {
        if (alignment == 0) return;
        lfs_size_t result = lfs_alignup(a, alignment);
        assert(result >= a && 
               "Property: alignup(a) must be >= a");
    };
    
    // ========================================
    // PROPERTY 3: Distance from input < alignment
    // ========================================
    auto check_distance = [](lfs_size_t a, lfs_size_t alignment) {
        if (alignment == 0) return;
        lfs_size_t result = lfs_alignup(a, alignment);
        assert((result - a) < alignment && 
               "Property: distance from input must be < alignment");
    };
    
    // ========================================
    // PROPERTY 4: Idempotence - alignup(alignup(a)) == alignup(a)
    // ========================================
    auto check_idempotence = [](lfs_size_t a, lfs_size_t alignment) {
        if (alignment == 0) return;
        lfs_size_t result1 = lfs_alignup(a, alignment);
        lfs_size_t result2 = lfs_alignup(result1, alignment);
        assert(result1 == result2 && 
               "Property: alignup must be idempotent");
    };
    
    // ========================================
    // PROPERTY 5: Already aligned values unchanged
    // ========================================
    auto check_aligned_unchanged = [](lfs_size_t multiple, lfs_size_t alignment) {
        if (alignment == 0) return;
        lfs_size_t aligned = multiple * alignment;
        assert(lfs_alignup(aligned, alignment) == aligned && 
               "Property: already aligned values must be unchanged");
    };
    
    // ========================================
    // PROPERTY 6: Relationship with aligndown
    // For aligned values: alignup(a) == aligndown(a)
    // For non-aligned: alignup(a) == aligndown(a) + alignment
    // ========================================
    auto check_aligndown_relationship = [](lfs_size_t a, lfs_size_t alignment) {
        if (alignment == 0) return;
        lfs_size_t up = lfs_alignup(a, alignment);
        lfs_size_t down = lfs_aligndown(a, alignment);
        if (a % alignment == 0) {
            assert(up == down && 
                   "Property: for aligned values, alignup == aligndown");
        } else {
            assert(up == down + alignment && 
                   "Property: for non-aligned, alignup == aligndown + alignment");
        }
    };
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: Basic alignment
    {
        assert(lfs_alignup(10, 4) == 12);
        assert(lfs_alignup(11, 4) == 12);
        assert(lfs_alignup(12, 4) == 12);
        check_multiple(10, 4);
        check_greater_or_equal(10, 4);
        check_distance(10, 4);
    }
    
    // Test 2: Already aligned values
    {
        assert(lfs_alignup(16, 4) == 16);
        assert(lfs_alignup(16, 8) == 16);
        assert(lfs_alignup(16, 16) == 16);
        check_aligned_unchanged(4, 4);
        check_aligned_unchanged(8, 4);
        check_idempotence(16, 4);
    }
    
    // Test 3: Zero value
    {
        assert(lfs_alignup(0, 4) == 0);
        assert(lfs_alignup(0, 8) == 0);
        assert(lfs_alignup(0, 16) == 0);
        check_multiple(0, 4);
        check_idempotence(0, 4);
    }
    
    // Test 4: Alignment of 1 (no change)
    {
        assert(lfs_alignup(10, 1) == 10);
        assert(lfs_alignup(99, 1) == 99);
        assert(lfs_alignup(1000, 1) == 1000);
        check_idempotence(10, 1);
    }
    
    // Test 5: Power of 2 alignments (common in filesystems)
    {
        assert(lfs_alignup(97, 16) == 112);
        assert(lfs_alignup(100, 32) == 128);
        assert(lfs_alignup(100, 64) == 128);
        assert(lfs_alignup(1000, 256) == 1024);
        check_multiple(97, 16);
        check_distance(97, 16);
    }
    
    // Test 6: Large values
    {
        assert(lfs_alignup(4095, 4096) == 4096);
        assert(lfs_alignup(4096, 4096) == 4096);
        assert(lfs_alignup(4097, 4096) == 8192);
        check_multiple(4095, 4096);
        check_greater_or_equal(4095, 4096);
    }
    
    // Test 7: Non-power-of-2 alignments
    {
        assert(lfs_alignup(100, 3) == 102);
        assert(lfs_alignup(100, 7) == 105);
        assert(lfs_alignup(100, 11) == 110);
        check_multiple(100, 3);
        check_distance(100, 3);
    }
    
    // Test 8: Boundary cases
    {
        assert(lfs_alignup(1, 2) == 2);
        assert(lfs_alignup(2, 2) == 2);
        assert(lfs_alignup(3, 2) == 4);
        check_idempotence(1, 2);
        check_idempotence(2, 2);
    }
    
    // Test 9: Relationship with aligndown
    {
        check_aligndown_relationship(10, 4);
        check_aligndown_relationship(16, 4);
        check_aligndown_relationship(100, 16);
        check_aligndown_relationship(4096, 4096);
    }
    
    // Test 10: Stress test - verify properties
    {
        lfs_size_t alignments[] = {1, 2, 4, 8, 16, 32, 64, 128, 256};
        for (auto align : alignments) {
            for (lfs_size_t val = 0; val < 1000; val += 13) {
                check_multiple(val, align);
                check_greater_or_equal(val, align);
                check_distance(val, align);
                check_idempotence(val, align);
                check_aligndown_relationship(val, align);
            }
        }
    }
    
    std::cout << "All lfs_alignup tests passed!" << std::endl;
    return 0;
}
