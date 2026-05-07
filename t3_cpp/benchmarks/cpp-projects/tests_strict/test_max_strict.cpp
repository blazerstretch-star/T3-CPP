#include "../src/ds_algorithms.h"
#include <cassert>
#include <climits>

int main() {
    // ========================================
    // PROPERTY DEFINITIONS
    // ========================================
    
    // Property 1: max(a,b) >= a AND max(a,b) >= b
    auto property_greater_equal = [](int a, int b, int result) {
        return result >= a && result >= b;
    };
    
    // Property 2: max(a,b) == a OR max(a,b) == b
    auto property_is_one_of = [](int a, int b, int result) {
        return result == a || result == b;
    };
    
    // Property 3: max(a,b) == max(b,a) (commutative)
    auto property_commutative = [](int a, int b) {
        return Algorithms::max(a, b) == Algorithms::max(b, a);
    };
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: Basic positive numbers
    {
        int a = 5, b = 3;
        int result = Algorithms::max(a, b);
        assert(property_greater_equal(a, b, result) && "max(5,3) must be >= both");
        assert(property_is_one_of(a, b, result) && "max(5,3) must be 5 or 3");
        assert(result == 5 && "max(5,3) should be 5");
    }
    
    // Test 2: Commutative property
    {
        int a = 3, b = 5;
        int result = Algorithms::max(a, b);
        assert(property_commutative(a, b) && "max must be commutative");
        assert(result == 5 && "max(3,5) should be 5");
    }
    
    // Test 3: Negative numbers
    {
        int a = -1, b = -5;
        int result = Algorithms::max(a, b);
        assert(property_greater_equal(a, b, result) && "max(-1,-5) must be >= both");
        assert(property_is_one_of(a, b, result) && "max(-1,-5) must be -1 or -5");
        assert(result == -1 && "max(-1,-5) should be -1");
    }
    
    // Test 4: Equal values
    {
        int a = 5, b = 5;
        int result = Algorithms::max(a, b);
        assert(property_greater_equal(a, b, result) && "max(5,5) must be >= both");
        assert(result == 5 && "max(5,5) should be 5");
    }
    
    // Test 5: Zero cases
    {
        assert(Algorithms::max(0, -1) == 0 && "max(0,-1) should be 0");
        assert(Algorithms::max(0, 1) == 1 && "max(0,1) should be 1");
        assert(Algorithms::max(0, 0) == 0 && "max(0,0) should be 0");
    }
    
    // Test 6: Mixed signs
    {
        int a = -100, b = 100;
        int result = Algorithms::max(a, b);
        assert(property_greater_equal(a, b, result) && "max(-100,100) must be >= both");
        assert(result == 100 && "max(-100,100) should be 100");
    }
    
    // Test 7: Boundary values
    {
        int result1 = Algorithms::max(INT_MAX, INT_MAX - 1);
        assert(result1 == INT_MAX && "max(INT_MAX, INT_MAX-1) should be INT_MAX");
        
        int result2 = Algorithms::max(INT_MIN, INT_MIN + 1);
        assert(result2 == INT_MIN + 1 && "max(INT_MIN, INT_MIN+1) should be INT_MIN+1");
        
        int result3 = Algorithms::max(INT_MIN, INT_MAX);
        assert(result3 == INT_MAX && "max(INT_MIN, INT_MAX) should be INT_MAX");
    }
    
    // Test 8: Large numbers
    {
        int a = 1000000, b = 999999;
        int result = Algorithms::max(a, b);
        assert(property_greater_equal(a, b, result) && "max must satisfy >= property");
        assert(property_commutative(a, b) && "max must be commutative");
        assert(result == 1000000 && "max(1000000,999999) should be 1000000");
    }
    
    // Test 9: Deterministic behavior
    {
        int a = 42, b = 17;
        int result1 = Algorithms::max(a, b);
        int result2 = Algorithms::max(a, b);
        assert(result1 == result2 && "max must be deterministic");
    }
    
    return 0;
}
