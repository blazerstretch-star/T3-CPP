#include "../src/numerical_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>
#include <limits>

int main() {
    using namespace NumericalMethods;
    
    // ========================================
    // PROPERTY 1: Positive numbers return positive value
    // ========================================
    assert(sign(5.0) > 0 && "Positive number should return positive sign");
    assert(sign(0.001) > 0 && "Small positive should return positive sign");
    assert(sign(1e10) > 0 && "Large positive should return positive sign");
    assert(sign(1e-100) > 0 && "Tiny positive should return positive sign");
    assert(sign(std::numeric_limits<double>::max()) > 0 && "Max double should be positive");
    
    // ========================================
    // PROPERTY 2: Negative numbers return negative value
    // ========================================
    assert(sign(-3.0) < 0 && "Negative number should return negative sign");
    assert(sign(-0.001) < 0 && "Small negative should return negative sign");
    assert(sign(-1e10) < 0 && "Large negative should return negative sign");
    assert(sign(-1e-100) < 0 && "Tiny negative should return negative sign");
    assert(sign(-std::numeric_limits<double>::max()) < 0 && "Negative max should be negative");
    
    // ========================================
    // PROPERTY 3: Zero handling (consistent behavior)
    // ========================================
    int sign_zero = sign(0.0);
    assert((sign_zero == 1 || sign_zero == -1) && "Zero should return either +1 or -1");
    assert(sign(0.0) == sign_zero && "Zero should return consistent value");
    assert(sign(-0.0) == sign_zero && "Negative zero should return same as positive zero");
    
    // ========================================
    // PROPERTY 4: Opposite signs for opposite values
    // ========================================
    double test_vals[] = {1.0, 5.5, 100.0, 0.001, 1e10, 42.7};
    for (double x : test_vals) {
        int s1 = sign(x);
        int s2 = sign(-x);
        assert(s1 * s2 < 0 && "sign(x) and sign(-x) should have opposite signs");
    }
    
    // ========================================
    // PROPERTY 5: Idempotence - sign doesn't change magnitude relationship
    // ========================================
    assert(sign(100.0) * sign(50.0) > 0 && "Same sign numbers should have same sign result");
    assert(sign(-100.0) * sign(-50.0) > 0 && "Same sign negatives should have same sign result");
    assert(sign(100.0) * sign(-50.0) < 0 && "Opposite sign numbers should have opposite sign result");
    
    std::cout << "test_sign PASSED" << std::endl;
    return 0;
}
