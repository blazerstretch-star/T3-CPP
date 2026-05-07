#include "../src/lfs_functions.h"
#include <cassert>
#include <iostream>

int main() {
    // ========================================
    // PROPERTY 1: Reflexive - pair compared with itself returns 0
    // ========================================
    auto check_reflexive = [](lfs_block_t a, lfs_block_t b) {
        lfs_block_t pair[2] = {a, b};
        assert(lfs_pair_cmp(pair, pair) == 0 && 
               "Property: pair_cmp(pair, pair) == 0 (reflexive)");
    };
    
    // ========================================
    // PROPERTY 2: Symmetric - cmp(a,b) == cmp(b,a)
    // ========================================
    auto check_symmetric = [](lfs_block_t a[2], lfs_block_t b[2]) {
        int result1 = lfs_pair_cmp(a, b);
        int result2 = lfs_pair_cmp(b, a);
        assert(result1 == result2 && 
               "Property: pair_cmp is symmetric");
    };
    
    // ========================================
    // PROPERTY 3: Returns 0 if pairs share ANY element
    // ========================================
    auto check_shared_element = [](lfs_block_t a1, lfs_block_t a2, 
                                     lfs_block_t b1, lfs_block_t b2) {
        lfs_block_t paira[2] = {a1, a2};
        lfs_block_t pairb[2] = {b1, b2};
        
        bool shares_element = (a1 == b1 || a1 == b2 || a2 == b1 || a2 == b2);
        int result = lfs_pair_cmp(paira, pairb);
        
        if (shares_element) {
            assert(result == 0 && 
                   "Property: pairs sharing any element should return 0");
        } else {
            assert(result != 0 && 
                   "Property: pairs with no shared elements should return non-zero");
        }
    };
    
    // ========================================
    // PROPERTY 4: Order within pair doesn't matter
    // ========================================
    auto check_order_independent = [](lfs_block_t a, lfs_block_t b,
                                       lfs_block_t c, lfs_block_t d) {
        lfs_block_t pair1[2] = {a, b};
        lfs_block_t pair2[2] = {b, a};
        lfs_block_t pair3[2] = {c, d};
        
        assert(lfs_pair_cmp(pair1, pair3) == lfs_pair_cmp(pair2, pair3) && 
               "Property: order within pair doesn't affect comparison");
    };
    
    // ========================================
    // PROPERTY 5: Deterministic
    // ========================================
    auto check_deterministic = [](lfs_block_t a[2], lfs_block_t b[2]) {
        int result1 = lfs_pair_cmp(a, b);
        int result2 = lfs_pair_cmp(a, b);
        assert(result1 == result2 && 
               "Property: pair_cmp is deterministic");
    };
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: Identical pairs
    {
        lfs_block_t pair1[2] = {10, 20};
        lfs_block_t pair2[2] = {10, 20};
        assert(lfs_pair_cmp(pair1, pair2) == 0);
        check_reflexive(10, 20);
    }
    
    // Test 2: Reversed pairs (same elements, different order)
    {
        lfs_block_t pair1[2] = {10, 20};
        lfs_block_t pair2[2] = {20, 10};
        assert(lfs_pair_cmp(pair1, pair2) == 0 && 
               "Pairs with same elements in different order should match");
    }
    
    // Test 3: Pairs sharing first element
    {
        lfs_block_t pair1[2] = {10, 20};
        lfs_block_t pair2[2] = {10, 30};
        assert(lfs_pair_cmp(pair1, pair2) == 0 && 
               "Pairs sharing element 10 should match");
        check_shared_element(10, 20, 10, 30);
    }
    
    // Test 4: Pairs sharing second element
    {
        lfs_block_t pair1[2] = {10, 20};
        lfs_block_t pair2[2] = {30, 20};
        assert(lfs_pair_cmp(pair1, pair2) == 0 && 
               "Pairs sharing element 20 should match");
        check_shared_element(10, 20, 30, 20);
    }
    
    // Test 5: Pairs sharing cross elements
    {
        lfs_block_t pair1[2] = {10, 20};
        lfs_block_t pair2[2] = {20, 30};
        assert(lfs_pair_cmp(pair1, pair2) == 0 && 
               "Pairs sharing element 20 (cross) should match");
        check_shared_element(10, 20, 20, 30);
    }
    
    // Test 6: No shared elements
    {
        lfs_block_t pair1[2] = {10, 20};
        lfs_block_t pair2[2] = {5, 15};
        assert(lfs_pair_cmp(pair1, pair2) != 0 && 
               "Pairs with no shared elements should not match");
        check_shared_element(10, 20, 5, 15);
    }
    
    // Test 7: Identical elements within pair
    {
        lfs_block_t pair1[2] = {10, 10};
        lfs_block_t pair2[2] = {10, 20};
        assert(lfs_pair_cmp(pair1, pair2) == 0 && 
               "Pairs sharing element 10 should match");
        
        lfs_block_t pair3[2] = {10, 10};
        lfs_block_t pair4[2] = {10, 10};
        assert(lfs_pair_cmp(pair3, pair4) == 0);
    }
    
    // Test 8: Zero values
    {
        lfs_block_t pair1[2] = {0, 10};
        lfs_block_t pair2[2] = {0, 20};
        assert(lfs_pair_cmp(pair1, pair2) == 0 && 
               "Pairs sharing element 0 should match");
        
        lfs_block_t pair3[2] = {0, 0};
        lfs_block_t pair4[2] = {0, 10};
        assert(lfs_pair_cmp(pair3, pair4) == 0);
        
        check_shared_element(0, 10, 0, 20);
    }
    
    // Test 9: Large values
    {
        lfs_block_t large1 = 0xFFFFFFFE;
        lfs_block_t large2 = 0x80000000;
        
        lfs_block_t pair1[2] = {large1, large2};
        lfs_block_t pair2[2] = {large1, 100};
        assert(lfs_pair_cmp(pair1, pair2) == 0);
        
        lfs_block_t pair3[2] = {100, 200};
        assert(lfs_pair_cmp(pair1, pair3) != 0);
        
        check_shared_element(large1, large2, large1, 100);
    }
    
    // Test 10: Special block values
    {
        lfs_block_t pair1[2] = {LFS_BLOCK_NULL, 10};
        lfs_block_t pair2[2] = {LFS_BLOCK_NULL, 20};
        assert(lfs_pair_cmp(pair1, pair2) == 0);
        
        lfs_block_t pair3[2] = {LFS_BLOCK_INLINE, 10};
        lfs_block_t pair4[2] = {LFS_BLOCK_INLINE, 30};
        assert(lfs_pair_cmp(pair3, pair4) == 0);
        
        check_shared_element(LFS_BLOCK_NULL, 10, LFS_BLOCK_NULL, 20);
    }
    
    // Test 11: Symmetry
    {
        lfs_block_t pair1[2] = {10, 20};
        lfs_block_t pair2[2] = {30, 40};
        check_symmetric(pair1, pair2);
        
        lfs_block_t pair3[2] = {10, 30};
        check_symmetric(pair1, pair3);
    }
    
    // Test 12: Order independence
    {
        check_order_independent(10, 20, 30, 40);
        check_order_independent(10, 20, 10, 30);
        check_order_independent(5, 15, 25, 35);
    }
    
    // Test 13: Reflexivity
    {
        check_reflexive(10, 20);
        check_reflexive(0, 0);
        check_reflexive(LFS_BLOCK_NULL, 100);
        check_reflexive(LFS_BLOCK_INLINE, 200);
    }
    
    // Test 14: Determinism
    {
        lfs_block_t pair1[2] = {10, 20};
        lfs_block_t pair2[2] = {30, 40};
        check_deterministic(pair1, pair2);
        
        lfs_block_t pair3[2] = {10, 30};
        check_deterministic(pair1, pair3);
    }
    
    // Test 15: Stress test - comprehensive combinations
    {
        lfs_block_t values[] = {0, 1, 10, 20, 50, 100, 200};
        
        for (auto a1 : values) {
            for (auto a2 : values) {
                for (auto b1 : values) {
                    for (auto b2 : values) {
                        check_shared_element(a1, a2, b1, b2);
                        
                        lfs_block_t paira[2] = {a1, a2};
                        lfs_block_t pairb[2] = {b1, b2};
                        check_symmetric(paira, pairb);
                        check_deterministic(paira, pairb);
                    }
                }
            }
        }
    }
    
    // Test 16: All different vs all same
    {
        // All different
        lfs_block_t pair1[2] = {1, 2};
        lfs_block_t pair2[2] = {3, 4};
        assert(lfs_pair_cmp(pair1, pair2) != 0);
        
        // All same
        lfs_block_t pair3[2] = {5, 5};
        lfs_block_t pair4[2] = {5, 5};
        assert(lfs_pair_cmp(pair3, pair4) == 0);
    }
    
    std::cout << "All lfs_pair_cmp tests passed!" << std::endl;
    return 0;
}
