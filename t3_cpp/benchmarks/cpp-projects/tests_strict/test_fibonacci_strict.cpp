#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    // Property 1: fib(0) = 0, fib(1) = 1
    // Property 2: fib(n) = fib(n-1) + fib(n-2) for n >= 2
    // Property 3: fib(n) >= 0 for all n >= 0
    // Property 4: fib(n) > fib(n-1) for n > 1
    
    auto verify_recursive_property = [](int n) {
        if (n < 2) return true;
        return Algorithms::fibonacci(n) == 
               Algorithms::fibonacci(n-1) + Algorithms::fibonacci(n-2);
    };
    
    // Test 1: Base cases
    assert(Algorithms::fibonacci(0) == 0 && "fib(0) = 0");
    assert(Algorithms::fibonacci(1) == 1 && "fib(1) = 1");
    
    // Test 2: Small values
    assert(Algorithms::fibonacci(2) == 1 && "fib(2) = 1");
    assert(Algorithms::fibonacci(3) == 2 && "fib(3) = 2");
    assert(Algorithms::fibonacci(4) == 3 && "fib(4) = 3");
    assert(Algorithms::fibonacci(5) == 5 && "fib(5) = 5");
    
    // Test 3: Medium values
    assert(Algorithms::fibonacci(6) == 8 && "fib(6) = 8");
    assert(Algorithms::fibonacci(7) == 13 && "fib(7) = 13");
    assert(Algorithms::fibonacci(8) == 21 && "fib(8) = 21");
    assert(Algorithms::fibonacci(9) == 34 && "fib(9) = 34");
    assert(Algorithms::fibonacci(10) == 55 && "fib(10) = 55");
    
    // Test 4: Recursive property
    assert(verify_recursive_property(5) && "fib(5) = fib(4) + fib(3)");
    assert(verify_recursive_property(10) && "fib(10) = fib(9) + fib(8)");
    assert(verify_recursive_property(15) && "fib(15) = fib(14) + fib(13)");
    
    // Test 5: Monotonic increasing (for n > 2, since fib(2) = fib(1) = 1)
    for (int i = 3; i <= 15; i++) {
        assert(Algorithms::fibonacci(i) > Algorithms::fibonacci(i-1) && 
               "Fibonacci is monotonic increasing for n > 2");
    }
    
    // Test 6: Known sequence
    int expected[] = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610};
    for (int i = 0; i < 16; i++) {
        assert(Algorithms::fibonacci(i) == expected[i] && "Fibonacci sequence correct");
    }
    
    // Test 7: Larger value
    assert(Algorithms::fibonacci(20) == 6765 && "fib(20) = 6765");
    
    // Test 8: Deterministic
    int result1 = Algorithms::fibonacci(12);
    int result2 = Algorithms::fibonacci(12);
    assert(result1 == result2 && "fibonacci is deterministic");
    
    return 0;
}
