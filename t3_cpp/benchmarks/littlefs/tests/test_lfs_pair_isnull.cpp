#include "../src/lfs_functions.h"
#include <cassert>
#include <iostream>

int main() {
    // ========================================
    // PROPERTY 1: Returns true if ANY element is NULL
    // ========================================
    auto check_any_null = [](lfs_block_t a, lfs_block_t b) {
        lfs_block_t pair[2] = {a, b};
        bool result = lfs_pair_isnull(pair);
        bool expected = (a == LFS_BLOCK_NULL || b == LFS_BLOCK_NULL);
        assert(result == expected && 
               "Property: isnull returns true if ANY element is NULL");
    };
    
    // ========================================
    // PROPERTY 2: Returns false only if BOTH elements are non-NULL
    // ========================================
    auto check_both_non_null = [](lfs_block_t a, lfs_block_t b) {
        if (a == LFS_BLOCK_NULL || b == LFS_BLOCK_NULL) return;
        lfs_block_t pair[2] = {a, b};
        assert(lfs_pair_isnull(pair) == false && 
               "Property: isnull returns false when both elements are non-NULL");
    };
    
    // ========================================
    // PROPERTY 3: Symmetric - order doesn't matter
    // ========================================
    auto check_symmetric = [](lfs_block_t a, lfs_block_t b) {
        lfs_block_t pair1[2] = {a, b};
        lfs_block_t pair2[2] = {b, a};
        assert(lfs_pair_isnull(pair1) == lfs_pair_isnull(pair2) && 
               "Property: isnull is symmetric (order independent)");
    };
    
    // ========================================
    // PROPERTY 4: Deterministic
    // ========================================
    auto check_deterministic = [](lfs_block_t a, lfs_block_t b) {
        lfs_block_t pair[2] = {a, b};
        bool result1 = lfs_pair_isnull(pair);
        bool result2 = lfs_pair_isnull(pair);
        assert(result1 == result2 && 
               "Property: isnull is deterministic");
    };
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: First element NULL
    {
        lfs_block_t pair[2] = {LFS_BLOCK_NULL, 10};
        assert(lfs_pair_isnull(pair) == true);
        check_any_null(LFS_BLOCK_NULL, 10);
    }
    
    // Test 2: Second element NULL
    {
        lfs_block_t pair[2] = {10, LFS_BLOCK_NULL};
        assert(lfs_pair_isnull(pair) == true);
        check_any_null(10, LFS_BLOCK_NULL);
    }
    
    // Test 3: Both elements NULL
    {
        lfs_block_t pair[2] = {LFS_BLOCK_NULL, LFS_BLOCK_NULL};
        assert(lfs_pair_isnull(pair) == true);
        check_any_null(LFS_BLOCK_NULL, LFS_BLOCK_NULL);
    }
    
    // Test 4: Neither element NULL
    {
        lfs_block_t pair[2] = {10, 20};
        assert(lfs_pair_isnull(pair) == false);
        check_both_non_null(10, 20);
    }
    
    // Test 5: Zero is not NULL
    {
        lfs_block_t pair1[2] = {0, 10};
        assert(lfs_pair_isnull(pair1) == false && 
               "Zero is a valid block number, not NULL");
        
        lfs_block_t pair2[2] = {0, 0};
        assert(lfs_pair_isnull(pair2) == false && 
               "Both zeros are valid, not NULL");
        
        check_both_non_null(0, 10);
        check_both_non_null(0, 0);
    }
    
    // Test 6: LFS_BLOCK_INLINE is not NULL
    {
        lfs_block_t pair1[2] = {LFS_BLOCK_INLINE, 10};
        assert(lfs_pair_isnull(pair1) == false && 
               "LFS_BLOCK_INLINE is valid, not NULL");
        
        lfs_block_t pair2[2] = {LFS_BLOCK_INLINE, LFS_BLOCK_INLINE};
        assert(lfs_pair_isnull(pair2) == false && 
               "Both INLINE blocks are valid, not NULL");
        
        check_both_non_null(LFS_BLOCK_INLINE, 10);
    }
    
    // Test 7: Mix of NULL and INLINE
    {
        lfs_block_t pair1[2] = {LFS_BLOCK_NULL, LFS_BLOCK_INLINE};
        assert(lfs_pair_isnull(pair1) == true);
        
        lfs_block_t pair2[2] = {LFS_BLOCK_INLINE, LFS_BLOCK_NULL};
        assert(lfs_pair_isnull(pair2) == true);
        
        check_any_null(LFS_BLOCK_NULL, LFS_BLOCK_INLINE);
    }
    
    // Test 8: Symmetry
    {
        check_symmetric(LFS_BLOCK_NULL, 10);
        check_symmetric(10, LFS_BLOCK_NULL);
        check_symmetric(10, 20);
        check_symmetric(LFS_BLOCK_NULL, LFS_BLOCK_NULL);
        check_symmetric(0, 100);
    }
    
    // Test 9: Large block numbers
    {
        lfs_block_t large = 0xFFFFFFFE; // Max valid block (NULL is 0xFFFFFFFF)
        
        lfs_block_t pair1[2] = {large, 100};
        assert(lfs_pair_isnull(pair1) == false);
        
        lfs_block_t pair2[2] = {large, LFS_BLOCK_NULL};
        assert(lfs_pair_isnull(pair2) == true);
        
        check_both_non_null(large, 100);
        check_any_null(large, LFS_BLOCK_NULL);
    }
    
    // Test 10: Determinism
    {
        check_deterministic(LFS_BLOCK_NULL, 10);
        check_deterministic(10, 20);
        check_deterministic(0, 0);
        check_deterministic(LFS_BLOCK_NULL, LFS_BLOCK_NULL);
    }
    
    // Test 11: Stress test - all combinations
    {
        lfs_block_t test_values[] = {
            0, 1, 10, 100, 1000,
            LFS_BLOCK_NULL,
            LFS_BLOCK_INLINE,
            0xFFFFFFFE
        };
        
        for (auto a : test_values) {
            for (auto b : test_values) {
                check_any_null(a, b);
                check_symmetric(a, b);
                check_deterministic(a, b);
                
                if (a != LFS_BLOCK_NULL && b != LFS_BLOCK_NULL) {
                    check_both_non_null(a, b);
                }
            }
        }
    }
    
    // Test 12: Edge cases with sequential blocks
    {
        for (lfs_block_t i = 0; i < 100; i++) {
            lfs_block_t pair[2] = {i, i + 1};
            assert(lfs_pair_isnull(pair) == false && 
                   "Sequential valid blocks should not be null");
            check_both_non_null(i, i + 1);
        }
    }
    
    // Test 13: Verify NULL constant value
    {
        // LFS_BLOCK_NULL should be 0xFFFFFFFF (all bits set)
        lfs_block_t pair1[2] = {0xFFFFFFFF, 10};
        assert(lfs_pair_isnull(pair1) == true && 
               "0xFFFFFFFF should be recognized as NULL");
        
        lfs_block_t pair2[2] = {10, 0xFFFFFFFF};
        assert(lfs_pair_isnull(pair2) == true && 
               "0xFFFFFFFF should be recognized as NULL");
    }
    
    std::cout << "All lfs_pair_isnull tests passed!" << std::endl;
    return 0;
}
