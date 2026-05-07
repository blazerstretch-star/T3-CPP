#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    // ========================================
    // PROPERTY DEFINITIONS
    // ========================================
    
    // Property 1: 0! = 1
    auto property_zero_factorial = []() {
        return Algorithms::factorial(0) == 1;
    };
    
    // Property 2: 1! = 1
    auto property_one_factorial = []() {
        return Algorithms::factorial(1) == 1;
    };
    
    // Property 3: n! = n * (n-1)! for n > 0
    auto property_recursive_definition = [](int n) {
        if (n <= 0) return true;
        return Algorithms::factorial(n) == n * Algorithms::factorial(n - 1);
    };
    
    // Property 4: n! > (n-1)! for n > 1
    auto property_monotonic_increasing = [](int n) {
        if (n <= 1) return true;
        return Algorithms::factorial(n) > Algorithms::factorial(n - 1);
    };
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: Base case - 0!
    {
        assert(property_zero_factorial() && "0! must equal 1");
        assert(Algorithms::factorial(0) == 1 && "0! = 1");
    }
    
    // Test 2: Base case - 1!
    {
        assert(property_one_factorial() && "1! must equal 1");
        assert(Algorithms::factorial(1) == 1 && "1! = 1");
    }
    
    // Test 3: Small factorials
    {
        assert(Algorithms::factorial(2) == 2 && "2! = 2");
        assert(Algorithms::factorial(3) == 6 && "3! = 6");
        assert(Algorithms::factorial(4) == 24 && "4! = 24");
        assert(Algorithms::factorial(5) == 120 && "5! = 120");
    }
    
    // Test 4: Medium factorials
    {
        assert(Algorithms::factorial(6) == 720 && "6! = 720");
        assert(Algorithms::factorial(7) == 5040 && "7! = 5040");
        assert(Algorithms::factorial(8) == 40320 && "8! = 40320");
    }
    
    // Test 5: Recursive definition property
    {
        assert(property_recursive_definition(3) && "3! = 3 * 2!");
        assert(property_recursive_definition(5) && "5! = 5 * 4!");
        assert(property_recursive_definition(7) && "7! = 7 * 6!");
    }
    
    // Test 6: Monotonic increasing property
    {
        assert(property_monotonic_increasing(2) && "2! > 1!");
        assert(property_monotonic_increasing(5) && "5! > 4!");
        assert(property_monotonic_increasing(10) && "10! > 9!");
    }
    
    // Test 7: Known values
    {
        assert(Algorithms::factorial(9) == 362880 && "9! = 362880");
        assert(Algorithms::factorial(10) == 3628800 && "10! = 3628800");
    }
    
    // Test 8: Deterministic behavior
    {
        int result1 = Algorithms::factorial(6);
        int result2 = Algorithms::factorial(6);
        assert(result1 == result2 && "factorial is deterministic");
        assert(result1 == 720 && "6! = 720");
    }
    
    // Test 9: Sequence verification
    {
        // Verify factorial sequence: 1, 1, 2, 6, 24, 120, 720...
        int expected[] = {1, 1, 2, 6, 24, 120, 720, 5040, 40320};
        for (int i = 0; i < 9; i++) {
            assert(Algorithms::factorial(i) == expected[i] && "Factorial sequence correct");
        }
    }
    
    return 0;
}
