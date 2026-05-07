#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    // ========================================
    // PROPERTY DEFINITIONS
    // ========================================
    
    // Property 1: Numbers <= 1 are not prime
    // Property 2: 2 is the only even prime
    // Property 3: A prime has no divisors except 1 and itself
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: Numbers <= 1 are not prime
    assert(!Algorithms::isPrime(0) && "0 is not prime");
    assert(!Algorithms::isPrime(1) && "1 is not prime");
    assert(!Algorithms::isPrime(-5) && "Negative numbers not prime");
    
    // Test 2: Small primes
    assert(Algorithms::isPrime(2) && "2 is prime");
    assert(Algorithms::isPrime(3) && "3 is prime");
    assert(Algorithms::isPrime(5) && "5 is prime");
    assert(Algorithms::isPrime(7) && "7 is prime");
    assert(Algorithms::isPrime(11) && "11 is prime");
    assert(Algorithms::isPrime(13) && "13 is prime");
    
    // Test 3: Small composites
    assert(!Algorithms::isPrime(4) && "4 is not prime");
    assert(!Algorithms::isPrime(6) && "6 is not prime");
    assert(!Algorithms::isPrime(8) && "8 is not prime");
    assert(!Algorithms::isPrime(9) && "9 is not prime");
    assert(!Algorithms::isPrime(10) && "10 is not prime");
    
    // Test 4: Larger primes
    assert(Algorithms::isPrime(17) && "17 is prime");
    assert(Algorithms::isPrime(19) && "19 is prime");
    assert(Algorithms::isPrime(23) && "23 is prime");
    assert(Algorithms::isPrime(29) && "29 is prime");
    assert(Algorithms::isPrime(31) && "31 is prime");
    assert(Algorithms::isPrime(97) && "97 is prime");
    
    // Test 5: Larger composites
    assert(!Algorithms::isPrime(15) && "15 is not prime");
    assert(!Algorithms::isPrime(21) && "21 is not prime");
    assert(!Algorithms::isPrime(25) && "25 is not prime");
    assert(!Algorithms::isPrime(100) && "100 is not prime");
    
    // Test 6: Perfect squares
    assert(!Algorithms::isPrime(49) && "49 = 7*7 is not prime");
    assert(!Algorithms::isPrime(121) && "121 = 11*11 is not prime");
    
    // Test 7: Large prime
    assert(Algorithms::isPrime(101) && "101 is prime");
    assert(Algorithms::isPrime(103) && "103 is prime");
    
    // Test 8: Even numbers (except 2)
    assert(!Algorithms::isPrime(50) && "50 is not prime");
    assert(!Algorithms::isPrime(100) && "100 is not prime");
    
    // Test 9: Deterministic
    assert(Algorithms::isPrime(17) == Algorithms::isPrime(17) && "Deterministic");
    
    return 0;
}
