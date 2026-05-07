#include "../src/numerical_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    using namespace NumericalMethods;
    
    unsigned int seed = 12345;
    unsigned int a = 1664525;
    unsigned int c = 1013904223;
    unsigned int m = 2147483648;
    
    // ========================================
    // PROPERTY 1: All values must be non-negative
    // ========================================
    for (int i = 0; i < 1000; i++) {
        double r = exponential_random(1.0, seed, a, c, m);
        assert(r >= 0.0 && "Exponential random must be non-negative");
    }
    
    // ========================================
    // PROPERTY 2: Statistical mean ≈ 1/λ
    // ========================================
    unsigned int seed2 = 54321;
    double sum = 0;
    int n = 10000;
    double lambda = 1.0;
    
    for (int i = 0; i < n; i++) {
        double r = exponential_random(lambda, seed2, a, c, m);
        assert(r >= 0.0 && "All values must be non-negative");
        sum += r;
    }
    
    double mean = sum / n;
    double expected_mean = 1.0 / lambda;
    assert(std::abs(mean - expected_mean) < 0.05 && "Mean should be close to 1/lambda");
    
    // ========================================
    // PROPERTY 3: Different lambda values
    // ========================================
    unsigned int seed3 = 98765;
    double sum3 = 0;
    double lambda3 = 2.0;
    int n3 = 10000;
    
    for (int i = 0; i < n3; i++) {
        double r = exponential_random(lambda3, seed3, a, c, m);
        assert(r >= 0.0 && "All values must be non-negative");
        sum3 += r;
    }
    
    double mean3 = sum3 / n3;
    double expected_mean3 = 1.0 / lambda3;
    assert(std::abs(mean3 - expected_mean3) < 0.05 && "Mean should be close to 1/2 = 0.5");
    
    // ========================================
    // PROPERTY 4: Small lambda (large mean)
    // ========================================
    unsigned int seed4 = 11111;
    double sum4 = 0;
    double lambda4 = 0.5;
    int n4 = 5000;
    
    for (int i = 0; i < n4; i++) {
        double r = exponential_random(lambda4, seed4, a, c, m);
        assert(r >= 0.0 && "All values must be non-negative");
        sum4 += r;
    }
    
    double mean4 = sum4 / n4;
    double expected_mean4 = 1.0 / lambda4;
    assert(std::abs(mean4 - expected_mean4) < 0.1 && "Mean should be close to 2.0");
    
    // ========================================
    // PROPERTY 5: Variance ≈ 1/λ²
    // ========================================
    unsigned int seed5 = 22222;
    double sum5 = 0;
    double sum_sq5 = 0;
    double lambda5 = 1.0;
    int n5 = 10000;
    
    for (int i = 0; i < n5; i++) {
        double r = exponential_random(lambda5, seed5, a, c, m);
        sum5 += r;
        sum_sq5 += r * r;
    }
    
    double mean5 = sum5 / n5;
    double variance5 = (sum_sq5 / n5) - (mean5 * mean5);
    double expected_variance = 1.0 / (lambda5 * lambda5);
    assert(std::abs(variance5 - expected_variance) < 0.1 && "Variance should be close to 1/lambda^2");
    
    // ========================================
    // PROPERTY 6: Deterministic behavior
    // ========================================
    unsigned int seed6a = 33333;
    unsigned int seed6b = 33333;
    double r1 = exponential_random(1.0, seed6a, a, c, m);
    double r2 = exponential_random(1.0, seed6b, a, c, m);
    assert(r1 == r2 && "Same seed should produce same value");
    
    // ========================================
    // PROPERTY 7: Different consecutive values
    // ========================================
    unsigned int seed7 = 44444;
    double prev = exponential_random(1.0, seed7, a, c, m);
    int different_count = 0;
    for (int i = 0; i < 100; i++) {
        double curr = exponential_random(1.0, seed7, a, c, m);
        if (std::abs(curr - prev) > 1e-10) different_count++;
        prev = curr;
    }
    assert(different_count > 90 && "Sequence should have mostly different values");
    
    // ========================================
    // PROPERTY 8: Memoryless property (statistical)
    // ========================================
    // P(X > s+t | X > s) = P(X > t)
    // Most values should be small, fewer large values
    unsigned int seed8 = 55555;
    int small_count = 0, large_count = 0;
    int n8 = 10000;
    
    for (int i = 0; i < n8; i++) {
        double r = exponential_random(1.0, seed8, a, c, m);
        if (r < 1.0) small_count++;
        else if (r > 3.0) large_count++;
    }
    
    // For lambda=1: P(X < 1) ≈ 0.632, P(X > 3) ≈ 0.05
    assert(small_count > 5500 && small_count < 7000 && "Most values should be < 1");
    assert(large_count < 1000 && "Few values should be > 3");
    
    // ========================================
    // PROPERTY 9: Median ≈ ln(2)/λ
    // ========================================
    unsigned int seed9 = 66666;
    std::vector<double> values;
    int n9 = 10000;
    
    for (int i = 0; i < n9; i++) {
        values.push_back(exponential_random(1.0, seed9, a, c, m));
    }
    
    std::sort(values.begin(), values.end());
    double median = values[n9 / 2];
    double expected_median = std::log(2.0) / 1.0;  // ln(2) ≈ 0.693
    assert(std::abs(median - expected_median) < 0.1 && "Median should be close to ln(2)");
    
    std::cout << "test_exponential_random PASSED" << std::endl;
    return 0;
}
