#include "../src/lfs_functions.h"
#include <cassert>
#include <iostream>
#include <limits>

int main() {
    // ========================================
    // PROPERTY 1: Commutativity - min(a,b) == min(b,a)
    // ========================================
    auto check_commutativity = [](lfs_size_t a, lfs_size_t b) {
        assert(lfs_min(a, b) == lfs_min(b, a) && 
               "Property: min must be commutative");
    };
    
    // ========================================
    // PROPERTY 2: Result <= both inputs
    // ========================================
    auto check_bounds = [](lfs_size_t a, lfs_size_t b) {
        lfs_size_t result = lfs_min(a, b);
        assert(result <= a && result <= b && 
               "Property: min(a,b) must be <= both a and b");
    };
    
    // ========================================
    // PROPERTY 3: Result equals one of the inputs
    // ========================================
    auto check_membership = [](lfs_size_t a, lfs_size_t b) {
        lfs_size_t result = lfs_min(a, b);
        assert((result == a || result == b) && 
               "Property: min(a,b) must equal either a or b");
    };
    
    // ========================================
    // PROPERTY 4: Idempotence - min(a,a) == a
    // ========================================
    auto check_idempotence = [](lfs_size_t a) {
        assert(lfs_min(a, a) == a && 
               "Property: min(a,a) must equal a");
    };
    
    // ========================================
    // PROPERTY 5: Associativity - min(min(a,b),c) == min(a,min(b,c))
    // ========================================
    auto check_associativity = [](lfs_size_t a, lfs_size_t b, lfs_size_t c) {
        assert(lfs_min(lfs_min(a, b), c) == lfs_min(a, lfs_min(b, c)) && 
               "Property: min must be associative");
    };
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: Basic cases
    {
        assert(lfs_min(5, 10) == 5);
        assert(lfs_min(10, 5) == 5);
        check_commutativity(5, 10);
        check_bounds(5, 10);
        check_membership(5, 10);
    }
    
    // Test 2: Equal values
    {
        assert(lfs_min(7, 7) == 7);
        check_idempotence(7);
        check_idempotence(0);
        check_idempotence(999);
    }
    
    // Test 3: Zero boundary
    {
        assert(lfs_min(0, 100) == 0);
        assert(lfs_min(100, 0) == 0);
        assert(lfs_min(0, 0) == 0);
        check_commutativity(0, 100);
        check_bounds(0, 100);
    }
    
    // Test 4: Large values
    {
        lfs_size_t large = std::numeric_limits<lfs_size_t>::max();
        assert(lfs_min(large, 1) == 1);
        assert(lfs_min(1, large) == 1);
        assert(lfs_min(large, large) == large);
        check_commutativity(large, 1);
        check_bounds(large, 1);
    }
    
    // Test 5: Powers of 2 (common in filesystem operations)
    {
        assert(lfs_min(256, 512) == 256);
        assert(lfs_min(1024, 2048) == 1024);
        assert(lfs_min(4096, 4096) == 4096);
        check_commutativity(256, 512);
        check_associativity(256, 512, 1024);
    }
    
    // Test 6: Sequential values
    {
        assert(lfs_min(99, 100) == 99);
        assert(lfs_min(100, 101) == 100);
        check_commutativity(99, 100);
        check_bounds(99, 100);
    }
    
    // Test 7: Associativity with multiple values
    {
        check_associativity(5, 10, 15);
        check_associativity(100, 50, 75);
        check_associativity(1, 2, 3);
    }
    
    // Test 8: Stress test - verify properties hold for many values
    {
        for (lfs_size_t i = 0; i < 100; i++) {
            for (lfs_size_t j = 0; j < 100; j++) {
                check_commutativity(i, j);
                check_bounds(i, j);
                check_membership(i, j);
            }
        }
    }
    
    std::cout << "All lfs_min tests passed!" << std::endl;
    return 0;
}
