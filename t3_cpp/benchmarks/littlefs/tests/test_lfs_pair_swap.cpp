#include "../src/lfs_functions.h"
#include <cassert>
#include <iostream>

int main() {
    // ========================================
    // PROPERTY 1: Double swap returns to original
    // ========================================
    auto check_involution = [](lfs_block_t a, lfs_block_t b) {
        lfs_block_t pair[2] = {a, b};
        lfs_block_t original[2] = {a, b};
        
        lfs_pair_swap(pair);
        lfs_pair_swap(pair);
        
        assert(pair[0] == original[0] && pair[1] == original[1] && 
               "Property: swap(swap(pair)) == pair (involution)");
    };
    
    // ========================================
    // PROPERTY 2: Elements are exchanged
    // ========================================
    auto check_exchange = [](lfs_block_t a, lfs_block_t b) {
        lfs_block_t pair[2] = {a, b};
        lfs_pair_swap(pair);
        
        assert(pair[0] == b && pair[1] == a && 
               "Property: swap exchanges elements");
    };
    
    // ========================================
    // PROPERTY 3: No data loss
    // ========================================
    auto check_no_data_loss = [](lfs_block_t a, lfs_block_t b) {
        lfs_block_t pair[2] = {a, b};
        lfs_pair_swap(pair);
        
        // Both original values must still be present
        assert((pair[0] == a || pair[0] == b) && 
               (pair[1] == a || pair[1] == b) && 
               "Property: no data loss during swap");
    };
    
    // ========================================
    // PROPERTY 4: Identical elements remain identical
    // ========================================
    auto check_identical_unchanged = [](lfs_block_t a) {
        lfs_block_t pair[2] = {a, a};
        lfs_pair_swap(pair);
        
        assert(pair[0] == a && pair[1] == a && 
               "Property: swapping identical elements has no effect");
    };
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: Basic swap
    {
        lfs_block_t pair[2] = {10, 20};
        lfs_pair_swap(pair);
        assert(pair[0] == 20 && pair[1] == 10);
        check_exchange(10, 20);
    }
    
    // Test 2: Double swap (involution)
    {
        lfs_block_t pair[2] = {10, 20};
        lfs_pair_swap(pair);
        assert(pair[0] == 20 && pair[1] == 10);
        lfs_pair_swap(pair);
        assert(pair[0] == 10 && pair[1] == 20);
        
        check_involution(10, 20);
        check_involution(100, 200);
    }
    
    // Test 3: Identical elements
    {
        lfs_block_t pair[2] = {5, 5};
        lfs_pair_swap(pair);
        assert(pair[0] == 5 && pair[1] == 5);
        
        check_identical_unchanged(5);
        check_identical_unchanged(0);
        check_identical_unchanged(999);
    }
    
    // Test 4: Zero values
    {
        lfs_block_t pair1[2] = {0, 10};
        lfs_pair_swap(pair1);
        assert(pair1[0] == 10 && pair1[1] == 0);
        
        lfs_block_t pair2[2] = {0, 0};
        lfs_pair_swap(pair2);
        assert(pair2[0] == 0 && pair2[1] == 0);
        
        check_exchange(0, 10);
        check_involution(0, 10);
    }
    
    // Test 5: Large values
    {
        lfs_block_t large1 = 0xFFFFFFFF;
        lfs_block_t large2 = 0x80000000;
        
        lfs_block_t pair[2] = {large1, large2};
        lfs_pair_swap(pair);
        assert(pair[0] == large2 && pair[1] == large1);
        
        check_exchange(large1, large2);
        check_involution(large1, large2);
    }
    
    // Test 6: Special block values
    {
        lfs_block_t pair1[2] = {LFS_BLOCK_NULL, 100};
        lfs_pair_swap(pair1);
        assert(pair1[0] == 100 && pair1[1] == LFS_BLOCK_NULL);
        
        lfs_block_t pair2[2] = {LFS_BLOCK_INLINE, 200};
        lfs_pair_swap(pair2);
        assert(pair2[0] == 200 && pair2[1] == LFS_BLOCK_INLINE);
        
        check_exchange(LFS_BLOCK_NULL, 100);
        check_involution(LFS_BLOCK_INLINE, 200);
    }
    
    // Test 7: Sequential values
    {
        lfs_block_t pair[2] = {99, 100};
        lfs_pair_swap(pair);
        assert(pair[0] == 100 && pair[1] == 99);
        
        check_exchange(99, 100);
        check_no_data_loss(99, 100);
    }
    
    // Test 8: No data loss verification
    {
        check_no_data_loss(10, 20);
        check_no_data_loss(0, 1);
        check_no_data_loss(1000, 2000);
    }
    
    // Test 9: Multiple swaps (odd/even)
    {
        lfs_block_t pair[2] = {10, 20};
        
        // Odd number of swaps
        for (int i = 0; i < 3; i++) {
            lfs_pair_swap(pair);
        }
        assert(pair[0] == 20 && pair[1] == 10);
        
        // Even number of swaps (back to original)
        lfs_pair_swap(pair);
        assert(pair[0] == 10 && pair[1] == 20);
    }
    
    // Test 10: Stress test - verify properties
    {
        for (lfs_block_t i = 0; i < 100; i++) {
            for (lfs_block_t j = 0; j < 100; j++) {
                check_involution(i, j);
                check_exchange(i, j);
                check_no_data_loss(i, j);
            }
        }
    }
    
    // Test 11: Power of 2 values (common in block addressing)
    {
        lfs_block_t powers[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024};
        for (size_t i = 0; i < sizeof(powers)/sizeof(powers[0]) - 1; i++) {
            check_exchange(powers[i], powers[i+1]);
            check_involution(powers[i], powers[i+1]);
        }
    }
    
    std::cout << "All lfs_pair_swap tests passed!" << std::endl;
    return 0;
}
