#include "../src/ds_algorithms.h"
#include <cassert>
#include <climits>

int main() {
    // ========================================
    // PROPERTY DEFINITIONS
    // ========================================
    
    // Property 1: After swap(a,b), a has old b value and b has old a value
    auto verify_swap = [](int old_a, int old_b, int new_a, int new_b) {
        return new_a == old_b && new_b == old_a;
    };
    
    // Property 2: swap(swap(a,b)) = identity (double swap returns to original)
    auto verify_double_swap = [](int& a, int& b) {
        int orig_a = a, orig_b = b;
        Algorithms::swap(a, b);
        Algorithms::swap(a, b);
        return a == orig_a && b == orig_b;
    };
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: Basic positive numbers
    {
        int a = 5, b = 3;
        int old_a = a, old_b = b;
        Algorithms::swap(a, b);
        assert(verify_swap(old_a, old_b, a, b) && "Values swapped correctly");
        assert(a == 3 && b == 5 && "5 and 3 swapped");
    }
    
    // Test 2: Negative numbers
    {
        int a = -10, b = -20;
        int old_a = a, old_b = b;
        Algorithms::swap(a, b);
        assert(verify_swap(old_a, old_b, a, b) && "Negative values swapped");
        assert(a == -20 && b == -10 && "-10 and -20 swapped");
    }
    
    // Test 3: Zero and positive
    {
        int a = 0, b = 42;
        int old_a = a, old_b = b;
        Algorithms::swap(a, b);
        assert(verify_swap(old_a, old_b, a, b) && "Zero swapped correctly");
        assert(a == 42 && b == 0 && "0 and 42 swapped");
    }
    
    // Test 4: Equal values
    {
        int a = 7, b = 7;
        int old_a = a, old_b = b;
        Algorithms::swap(a, b);
        assert(verify_swap(old_a, old_b, a, b) && "Equal values swapped");
        assert(a == 7 && b == 7 && "Equal values remain equal");
    }
    
    // Test 5: Large numbers
    {
        int a = 1000000, b = 999999;
        int old_a = a, old_b = b;
        Algorithms::swap(a, b);
        assert(verify_swap(old_a, old_b, a, b) && "Large values swapped");
        assert(a == 999999 && b == 1000000 && "Large numbers swapped");
    }
    
    // Test 6: Mixed signs
    {
        int a = -100, b = 100;
        int old_a = a, old_b = b;
        Algorithms::swap(a, b);
        assert(verify_swap(old_a, old_b, a, b) && "Mixed signs swapped");
        assert(a == 100 && b == -100 && "Mixed signs swapped");
    }
    
    // Test 7: Double swap (identity property)
    {
        int a = 42, b = 17;
        assert(verify_double_swap(a, b) && "Double swap returns to original");
    }
    
    // Test 8: Boundary values
    {
        int a = INT_MAX, b = INT_MIN;
        int old_a = a, old_b = b;
        Algorithms::swap(a, b);
        assert(verify_swap(old_a, old_b, a, b) && "Boundary values swapped");
        assert(a == INT_MIN && b == INT_MAX && "INT_MAX and INT_MIN swapped");
    }
    
    // Test 9: Deterministic behavior
    {
        int a1 = 10, b1 = 20;
        int a2 = 10, b2 = 20;
        Algorithms::swap(a1, b1);
        Algorithms::swap(a2, b2);
        assert(a1 == a2 && b1 == b2 && "Swap is deterministic");
    }
    
    return 0;
}
