#include "../src/ds_algorithms.h"
#include <cassert>
#include <climits>

int main() {
    // ========================================
    // PROPERTY DEFINITIONS
    // ========================================
    
    // Property 1: abs(x) >= 0 for all x
    auto property_non_negative = [](int result) {
        return result >= 0;
    };
    
    // Property 2: abs(x) == x if x >= 0
    auto property_positive_unchanged = [](int x, int result) {
        if (x >= 0) return result == x;
        return true;
    };
    
    // Property 3: abs(x) == -x if x < 0
    auto property_negative_negated = [](int x, int result) {
        if (x < 0) return result == -x;
        return true;
    };
    
    // Property 4: abs(abs(x)) == abs(x) (idempotent)
    auto property_idempotent = [](int x) {
        int abs1 = Algorithms::absolute(x);
        int abs2 = Algorithms::absolute(abs1);
        return abs1 == abs2;
    };
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: Positive number
    {
        int x = 42;
        int result = Algorithms::absolute(x);
        assert(property_non_negative(result) && "Result must be non-negative");
        assert(property_positive_unchanged(x, result) && "Positive unchanged");
        assert(result == 42 && "abs(42) = 42");
    }
    
    // Test 2: Negative number
    {
        int x = -42;
        int result = Algorithms::absolute(x);
        assert(property_non_negative(result) && "Result must be non-negative");
        assert(property_negative_negated(x, result) && "Negative negated");
        assert(result == 42 && "abs(-42) = 42");
    }
    
    // Test 3: Zero
    {
        int x = 0;
        int result = Algorithms::absolute(x);
        assert(property_non_negative(result) && "Result must be non-negative");
        assert(result == 0 && "abs(0) = 0");
    }
    
    // Test 4: Small negative
    {
        int x = -1;
        int result = Algorithms::absolute(x);
        assert(property_non_negative(result) && "Result must be non-negative");
        assert(result == 1 && "abs(-1) = 1");
    }
    
    // Test 5: Large positive
    {
        int x = 1000000;
        int result = Algorithms::absolute(x);
        assert(property_non_negative(result) && "Result must be non-negative");
        assert(result == 1000000 && "abs(1000000) = 1000000");
    }
    
    // Test 6: Large negative
    {
        int x = -1000000;
        int result = Algorithms::absolute(x);
        assert(property_non_negative(result) && "Result must be non-negative");
        assert(result == 1000000 && "abs(-1000000) = 1000000");
    }
    
    // Test 7: Idempotent property
    {
        assert(property_idempotent(42) && "abs is idempotent for positive");
        assert(property_idempotent(-42) && "abs is idempotent for negative");
        assert(property_idempotent(0) && "abs is idempotent for zero");
    }
    
    // Test 8: Boundary value (INT_MAX)
    {
        int x = INT_MAX;
        int result = Algorithms::absolute(x);
        assert(property_non_negative(result) && "Result must be non-negative");
        assert(result == INT_MAX && "abs(INT_MAX) = INT_MAX");
    }
    
    // Test 9: Deterministic behavior
    {
        int x = -123;
        int result1 = Algorithms::absolute(x);
        int result2 = Algorithms::absolute(x);
        assert(result1 == result2 && "abs is deterministic");
    }
    
    return 0;
}
