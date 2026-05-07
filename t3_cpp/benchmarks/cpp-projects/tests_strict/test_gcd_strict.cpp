#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    // ========================================
    // PROPERTY DEFINITIONS
    // ========================================
    
    // Property 1: gcd(a, 0) = |a|
    // Property 2: gcd(a, b) = gcd(b, a) (commutative)
    // Property 3: gcd(a, b) divides both a and b
    // Property 4: gcd(a, a) = |a|
    
    auto divides = [](int divisor, int number) {
        if (divisor == 0) return number == 0;
        return number % divisor == 0;
    };
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: gcd with zero
    assert(Algorithms::gcd(5, 0) == 5 && "gcd(5, 0) = 5");
    assert(Algorithms::gcd(0, 7) == 7 && "gcd(0, 7) = 7");
    assert(Algorithms::gcd(0, 0) == 0 && "gcd(0, 0) = 0");
    
    // Test 2: gcd of equal numbers
    assert(Algorithms::gcd(5, 5) == 5 && "gcd(5, 5) = 5");
    assert(Algorithms::gcd(12, 12) == 12 && "gcd(12, 12) = 12");
    
    // Test 3: Coprime numbers (gcd = 1)
    assert(Algorithms::gcd(7, 11) == 1 && "gcd(7, 11) = 1");
    assert(Algorithms::gcd(13, 17) == 1 && "gcd(13, 17) = 1");
    assert(Algorithms::gcd(9, 16) == 1 && "gcd(9, 16) = 1");
    
    // Test 4: Common divisors
    assert(Algorithms::gcd(12, 18) == 6 && "gcd(12, 18) = 6");
    assert(Algorithms::gcd(24, 36) == 12 && "gcd(24, 36) = 12");
    assert(Algorithms::gcd(48, 18) == 6 && "gcd(48, 18) = 6");
    
    // Test 5: Commutative property
    assert(Algorithms::gcd(12, 18) == Algorithms::gcd(18, 12) && "Commutative");
    assert(Algorithms::gcd(7, 21) == Algorithms::gcd(21, 7) && "Commutative");
    
    // Test 6: Divisibility property
    {
        int a = 48, b = 18;
        int g = Algorithms::gcd(a, b);
        assert(divides(g, a) && divides(g, b) && "gcd divides both numbers");
    }
    
    // Test 7: One number divides the other
    assert(Algorithms::gcd(12, 24) == 12 && "gcd(12, 24) = 12");
    assert(Algorithms::gcd(5, 25) == 5 && "gcd(5, 25) = 5");
    
    // Test 8: Large numbers
    assert(Algorithms::gcd(1000, 500) == 500 && "gcd(1000, 500) = 500");
    assert(Algorithms::gcd(1071, 462) == 21 && "gcd(1071, 462) = 21");
    
    // Test 9: Deterministic
    assert(Algorithms::gcd(48, 18) == Algorithms::gcd(48, 18) && "Deterministic");
    
    return 0;
}
