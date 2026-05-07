#include "../src/lfs_functions.h"
#include <cassert>
#include <iostream>

int main() {
    // ========================================
    // PROPERTY 1: Result is 11-bit value (0x000 to 0x7FF)
    // ========================================
    auto check_11bit_range = [](lfs_tag_t tag) {
        uint16_t result = lfs_tag_type3(tag);
        assert(result <= 0x7FF && 
               "Property: type3 must be 11-bit value (0x000-0x7FF)");
    };
    
    // ========================================
    // PROPERTY 2: Only bits 20-30 affect result
    // ========================================
    auto check_bit_isolation = [](lfs_tag_t tag) {
        // Mask out bits 20-30
        lfs_tag_t masked = tag & 0x7FF00000;
        
        // Result should be same for tag and masked version
        assert(lfs_tag_type3(tag) == lfs_tag_type3(masked) && 
               "Property: only bits 20-30 should affect result");
    };
    
    // ========================================
    // PROPERTY 3: Lower bits don't affect result
    // ========================================
    auto check_lower_bits_ignored = [](lfs_tag_t tag) {
        lfs_tag_t modified_lower = tag ^ 0x000FFFFF; // Flip bits 0-19
        assert(lfs_tag_type3(tag) == lfs_tag_type3(modified_lower) && 
               "Property: lower 20 bits should not affect result");
    };
    
    // ========================================
    // PROPERTY 4: Upper bit (31) doesn't affect result
    // ========================================
    auto check_upper_bit_ignored = [](lfs_tag_t tag) {
        lfs_tag_t modified_upper = tag ^ 0x80000000; // Flip bit 31
        assert(lfs_tag_type3(tag) == lfs_tag_type3(modified_upper) && 
               "Property: bit 31 should not affect result");
    };
    
    // ========================================
    // PROPERTY 5: Deterministic
    // ========================================
    auto check_deterministic = [](lfs_tag_t tag) {
        uint16_t result1 = lfs_tag_type3(tag);
        uint16_t result2 = lfs_tag_type3(tag);
        assert(result1 == result2 && 
               "Property: tag_type3 is deterministic");
    };
    
    // ========================================
    // PROPERTY 6: Mathematical relationship - result == (tag & 0x7FF00000) >> 20
    // ========================================
    auto check_mathematical_formula = [](lfs_tag_t tag) {
        uint16_t result = lfs_tag_type3(tag);
        uint16_t expected = (tag & 0x7FF00000) >> 20;
        assert(result == expected && 
               "Property: type3 == (tag & 0x7FF00000) >> 20");
    };
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: Known test vectors
    {
        lfs_tag_t tag1 = 0x12345678;
        assert(lfs_tag_type3(tag1) == 0x123);
        
        lfs_tag_t tag2 = 0x7ff00000;
        assert(lfs_tag_type3(tag2) == 0x7ff);
        
        lfs_tag_t tag3 = 0x00000000;
        assert(lfs_tag_type3(tag3) == 0x000);
        
        lfs_tag_t tag4 = 0xabcdef12;
        assert(lfs_tag_type3(tag4) == 0x2bc);
    }
    
    // Test 2: All zeros
    {
        assert(lfs_tag_type3(0x00000000) == 0x000);
        check_11bit_range(0x00000000);
        check_deterministic(0x00000000);
    }
    
    // Test 3: All ones
    {
        assert(lfs_tag_type3(0xFFFFFFFF) == 0x7FF);
        check_11bit_range(0xFFFFFFFF);
        check_deterministic(0xFFFFFFFF);
    }
    
    // Test 4: Only type3 bits set
    {
        lfs_tag_t tag = 0x7FF00000; // Only bits 20-30 set
        assert(lfs_tag_type3(tag) == 0x7FF);
        check_bit_isolation(tag);
    }
    
    // Test 5: Only lower bits set (should return 0)
    {
        lfs_tag_t tag = 0x000FFFFF; // Only bits 0-19 set
        assert(lfs_tag_type3(tag) == 0x000);
        check_lower_bits_ignored(0x12300000);
    }
    
    // Test 6: Only upper bit set (should return 0)
    {
        lfs_tag_t tag = 0x80000000; // Only bit 31 set
        assert(lfs_tag_type3(tag) == 0x000);
        check_upper_bit_ignored(0x12300000);
    }
    
    // Test 7: Bit isolation - lower bits don't matter
    {
        lfs_tag_t base = 0x12300000;
        assert(lfs_tag_type3(base | 0x00000000) == 0x123);
        assert(lfs_tag_type3(base | 0x000FFFFF) == 0x123);
        assert(lfs_tag_type3(base | 0x00012345) == 0x123);
        check_lower_bits_ignored(base);
    }
    
    // Test 8: Bit isolation - upper bit doesn't matter
    {
        lfs_tag_t base = 0x12300000;
        assert(lfs_tag_type3(base) == lfs_tag_type3(base | 0x80000000));
        check_upper_bit_ignored(base);
    }
    
    // Test 9: Powers of 2 in type3 field
    {
        assert(lfs_tag_type3(0x00100000) == 0x001); // 2^0
        assert(lfs_tag_type3(0x00200000) == 0x002); // 2^1
        assert(lfs_tag_type3(0x00400000) == 0x004); // 2^2
        assert(lfs_tag_type3(0x00800000) == 0x008); // 2^3
        assert(lfs_tag_type3(0x01000000) == 0x010); // 2^4
        assert(lfs_tag_type3(0x02000000) == 0x020); // 2^5
        assert(lfs_tag_type3(0x04000000) == 0x040); // 2^6
        assert(lfs_tag_type3(0x08000000) == 0x080); // 2^7
        assert(lfs_tag_type3(0x10000000) == 0x100); // 2^8
        assert(lfs_tag_type3(0x20000000) == 0x200); // 2^9
        assert(lfs_tag_type3(0x40000000) == 0x400); // 2^10
    }
    
    // Test 10: Sequential type3 values
    {
        for (uint16_t i = 0; i <= 0x7FF; i++) {
            lfs_tag_t tag = ((uint32_t)i) << 20;
            assert(lfs_tag_type3(tag) == i);
            check_11bit_range(tag);
        }
    }
    
    // Test 11: Mathematical formula verification
    {
        check_mathematical_formula(0x12345678);
        check_mathematical_formula(0xFFFFFFFF);
        check_mathematical_formula(0x00000000);
        check_mathematical_formula(0x7FF00000);
        check_mathematical_formula(0xABCDEF12);
    }
    
    // Test 12: Stress test - random-like patterns
    {
        lfs_tag_t patterns[] = {
            0x00000000, 0xFFFFFFFF, 0x55555555, 0xAAAAAAAA,
            0x12345678, 0x87654321, 0xDEADBEEF, 0xCAFEBABE,
            0x11111111, 0x22222222, 0x44444444, 0x88888888
        };
        
        for (auto tag : patterns) {
            check_11bit_range(tag);
            check_bit_isolation(tag);
            check_lower_bits_ignored(tag);
            check_upper_bit_ignored(tag);
            check_deterministic(tag);
            check_mathematical_formula(tag);
        }
    }
    
    // Test 13: Boundary values in type3 field
    {
        // Minimum type3 value
        assert(lfs_tag_type3(0x00000000) == 0x000);
        
        // Maximum type3 value
        assert(lfs_tag_type3(0x7FF00000) == 0x7FF);
        
        // Just below maximum
        assert(lfs_tag_type3(0x7FE00000) == 0x7FE);
        
        // Just above minimum
        assert(lfs_tag_type3(0x00100000) == 0x001);
    }
    
    // Test 14: Verify result range
    {
        for (uint32_t i = 0; i < 10000; i++) {
            lfs_tag_t tag = i * 0x12345; // Pseudo-random pattern
            check_11bit_range(tag);
        }
    }
    
    // Test 15: Bit manipulation properties
    {
        lfs_tag_t base = 0x12300000;
        
        // XOR with lower bits shouldn't change result
        assert(lfs_tag_type3(base ^ 0x000FFFFF) == lfs_tag_type3(base));
        
        // XOR with upper bit shouldn't change result
        assert(lfs_tag_type3(base ^ 0x80000000) == lfs_tag_type3(base));
        
        // XOR with type3 bits should change result
        assert(lfs_tag_type3(base ^ 0x00100000) != lfs_tag_type3(base));
    }
    
    std::cout << "All lfs_tag_type3 tests passed!" << std::endl;
    return 0;
}
