#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    // ========================================
    // PROPERTY DEFINITIONS
    // ========================================
    
    // Property 1: x^0 = 1 for any x
    auto property_zero_exponent = [](int x) {
        return Algorithms::power(x, 0) == 1;
    };
    
    // Property 2: x^1 = x for any x
    auto property_one_exponent = [](int x) {
        return Algorithms::power(x, 1) == x;
    };
    
    // Property 3: 1^n = 1 for any n
    auto property_one_base = [](int n) {
        return Algorithms::power(1, n) == 1;
    };
    
    // Property 4: x^(a+b) = x^a * x^b (for small values to avoid overflow)
    auto property_exponent_addition = [](int x, int a, int b) {
        if (x == 0 || x == 1 || a + b > 10) return true; // Skip to avoid overflow
        int left = Algorithms::power(x, a + b);
        int right = Algorithms::power(x, a) * Algorithms::power(x, b);
        return left == right;
    };
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: Zero exponent
    {
        assert(property_zero_exponent(2) && "2^0 = 1");
        assert(property_zero_exponent(10) && "10^0 = 1");
        assert(property_zero_exponent(-5) && "(-5)^0 = 1");
        assert(Algorithms::power(0, 0) == 1 && "0^0 = 1 (by convention)");
    }
    
    // Test 2: One exponent
    {
        assert(property_one_exponent(5) && "5^1 = 5");
        assert(property_one_exponent(100) && "100^1 = 100");
        assert(property_one_exponent(-7) && "(-7)^1 = -7");
    }
    
    // Test 3: One base
    {
        assert(property_one_base(0) && "1^0 = 1");
        assert(property_one_base(1) && "1^1 = 1");
        assert(property_one_base(10) && "1^10 = 1");
        assert(property_one_base(100) && "1^100 = 1");
    }
    
    // Test 4: Basic powers
    {
        assert(Algorithms::power(2, 3) == 8 && "2^3 = 8");
        assert(Algorithms::power(3, 2) == 9 && "3^2 = 9");
        assert(Algorithms::power(5, 2) == 25 && "5^2 = 25");
        assert(Algorithms::power(10, 2) == 100 && "10^2 = 100");
    }
    
    // Test 5: Negative base with even exponent
    {
        assert(Algorithms::power(-2, 2) == 4 && "(-2)^2 = 4");
        assert(Algorithms::power(-3, 2) == 9 && "(-3)^2 = 9");
        assert(Algorithms::power(-5, 4) == 625 && "(-5)^4 = 625");
    }
    
    // Test 6: Negative base with odd exponent
    {
        assert(Algorithms::power(-2, 3) == -8 && "(-2)^3 = -8");
        assert(Algorithms::power(-3, 3) == -27 && "(-3)^3 = -27");
        assert(Algorithms::power(-1, 5) == -1 && "(-1)^5 = -1");
    }
    
    // Test 7: Zero base
    {
        assert(Algorithms::power(0, 1) == 0 && "0^1 = 0");
        assert(Algorithms::power(0, 5) == 0 && "0^5 = 0");
        assert(Algorithms::power(0, 100) == 0 && "0^100 = 0");
    }
    
    // Test 8: Exponent addition property
    {
        assert(property_exponent_addition(2, 2, 3) && "2^(2+3) = 2^2 * 2^3");
        assert(property_exponent_addition(3, 1, 2) && "3^(1+2) = 3^1 * 3^2");
    }
    
    // Test 9: Deterministic behavior
    {
        int result1 = Algorithms::power(7, 3);
        int result2 = Algorithms::power(7, 3);
        assert(result1 == result2 && "power is deterministic");
        assert(result1 == 343 && "7^3 = 343");
    }
    
    return 0;
}
