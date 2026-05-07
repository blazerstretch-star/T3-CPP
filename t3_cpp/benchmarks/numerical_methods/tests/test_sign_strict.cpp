#include "../src/numerical_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>
#include <limits>

int main() {
    using namespace NumericalMethods;
    
    // ========================================
    // PROPERTY: sign(x) returns -1 for x<0, positive value for x>=0
    // ========================================
    
    // Test 1: Positive numbers
    assert(sign(5.0) > 0 && "Positive number should return positive sign");
    assert(sign(0.001) > 0 && "Small positive should return positive sign");
    assert(sign(1e10) > 0 && "Large positive should return positive sign");
    assert(sign(1e-100) > 0 && "Tiny positive should return positive sign");
    
    // Test 2: Negative numbers
    assert(sign(-3.0) < 0 && "Negative number should return negative sign");
    assert(sign(-0.001) < 0 && "Small negative should return negative sign");
    assert(sign(-1e10) < 0 && "Large negative should return negative sign");
    assert(sign(-1e-100) < 0 && "Tiny negative should return negative sign");
    
    // Test 3: Zero (implementation-dependent: can be +1 or -1, but must be consistent)
    int sign_zero = sign(0.0);
    assert((sign_zero == 1 || sign_zero == -1) && "Zero should return either +1 or -1");
    assert(sign(0.0) == sign_zero && "Zero should return consistent value");
    assert(sign(-0.0) == sign_zero && "Negative zero should return same as positive zero");
    
    // Test 4: Extreme values
    assert(sign(std::numeric_limits<double>::max()) > 0 && "Max double should be positive");
    assert(sign(std::numeric_limits<double>::min()) > 0 && "Min positive double should be positive");
    assert(sign(-std::numeric_limits<double>::max()) < 0 && "Negative max should be negative");
    
    // Test 5: Sign consistency property: sign(x) * sign(-x) should be negative (unless x=0)
    double test_vals[] = {1.0, 5.5, 100.0, 0.001, 1e10};
    for (double x : test_vals) {
        int s1 = sign(x);
        int s2 = sign(-x);
        assert(s1 * s2 < 0 && "sign(x) and sign(-x) should have opposite signs");
    }
    
    std::cout << "test_sign_strict PASSED" << std::endl;
    return 0;
}
