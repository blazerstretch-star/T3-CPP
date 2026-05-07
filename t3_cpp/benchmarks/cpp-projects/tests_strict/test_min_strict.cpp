#include "../src/ds_algorithms.h"
#include <cassert>
#include <climits>

int main() {
    // ========================================
    // PROPERTY DEFINITIONS
    // ========================================
    
    // Property 1: min(a,b) <= a AND min(a,b) <= b
    auto property_less_equal = [](int a, int b, int result) {
        return result <= a && result <= b;
    };
    
    // Property 2: min(a,b) == a OR min(a,b) == b
    auto property_is_one_of = [](int a, int b, int result) {
        return result == a || result == b;
    };
    
    // Property 3: min(a,b) == min(b,a) (commutative)
    auto property_commutative = [](int a, int b) {
        return Algorithms::min(a, b) == Algorithms::min(b, a);
    };
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: Basic positive numbers
    {
        int a = 5, b = 3;
        int result = Algorithms::min(a, b);
        assert(property_less_equal(a, b, result) && "min(5,3) must be <= both");
        assert(property_is_one_of(a, b, result) && "min(5,3) must be 5 or 3");
        assert(result == 3 && "min(5,3) should be 3");
    }
    
    // Test 2: Commutative property
    {
        int a = 3, b = 5;
        int result = Algorithms::min(a, b);
        assert(property_commutative(a, b) && "min must be commutative");
        assert(result == 3 && "min(3,5) should be 3");
    }
    
    // Test 3: Negative numbers
    {
        int a = -1, b = -5;
        int result = Algorithms::min(a, b);
        assert(property_less_equal(a, b, result) && "min(-1,-5) must be <= both");
        assert(property_is_one_of(a, b, result) && "min(-1,-5) must be -1 or -5");
        assert(result == -5 && "min(-1,-5) should be -5");
    }
    
    // Test 4: Equal values
    {
        int a = 5, b = 5;
        int result = Algorithms::min(a, b);
        assert(property_less_equal(a, b, result) && "min(5,5) must be <= both");
        assert(result == 5 && "min(5,5) should be 5");
    }
    
    // Test 5: Zero cases
    {
        assert(Algorithms::min(0, -1) == -1 && "min(0,-1) should be -1");
        assert(Algorithms::min(0, 1) == 0 && "min(0,1) should be 0");
        assert(Algorithms::min(0, 0) == 0 && "min(0,0) should be 0");
    }
    
    // Test 6: Mixed signs
    {
        int a = -100, b = 100;
        int result = Algorithms::min(a, b);
        assert(property_less_equal(a, b, result) && "min(-100,100) must be <= both");
        assert(result == -100 && "min(-100,100) should be -100");
    }
    
    // Test 7: Boundary values
    {
        int result1 = Algorithms::min(INT_MAX, INT_MAX - 1);
        assert(result1 == INT_MAX - 1 && "min(INT_MAX, INT_MAX-1) should be INT_MAX-1");
        
        int result2 = Algorithms::min(INT_MIN, INT_MIN + 1);
        assert(result2 == INT_MIN && "min(INT_MIN, INT_MIN+1) should be INT_MIN");
        
        int result3 = Algorithms::min(INT_MIN, INT_MAX);
        assert(result3 == INT_MIN && "min(INT_MIN, INT_MAX) should be INT_MIN");
    }
    
    // Test 8: Large numbers
    {
        int a = 1000000, b = 999999;
        int result = Algorithms::min(a, b);
        assert(property_less_equal(a, b, result) && "min must satisfy <= property");
        assert(property_commutative(a, b) && "min must be commutative");
        assert(result == 999999 && "min(1000000,999999) should be 999999");
    }
    
    // Test 9: Deterministic behavior
    {
        int a = 42, b = 17;
        int result1 = Algorithms::min(a, b);
        int result2 = Algorithms::min(a, b);
        assert(result1 == result2 && "min must be deterministic");
    }
    
    return 0;
}
