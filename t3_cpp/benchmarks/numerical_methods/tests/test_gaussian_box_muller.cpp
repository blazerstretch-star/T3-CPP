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
    // PROPERTY 1: Statistical mean ≈ theoretical mean (μ)
    // ========================================
    double sum = 0;
    int n = 10000;
    double expected_mean = 0.0;
    double expected_sigma = 1.0;
    
    for (int i = 0; i < n; i++) {
        double r = gaussian_box_muller(expected_mean, expected_sigma, seed, a, c, m);
        sum += r;
    }
    
    double sample_mean = sum / n;
    assert(std::abs(sample_mean - expected_mean) < 0.05 && "Sample mean should be close to 0");
    
    // ========================================
    // PROPERTY 2: Statistical variance ≈ theoretical variance (σ²)
    // ========================================
    unsigned int seed2 = 54321;
    double sum2 = 0;
    double sum_sq = 0;
    
    for (int i = 0; i < n; i++) {
        double r = gaussian_box_muller(0.0, 1.0, seed2, a, c, m);
        sum2 += r;
        sum_sq += r * r;
    }
    
    double mean2 = sum2 / n;
    double variance = (sum_sq / n) - (mean2 * mean2);
    assert(std::abs(variance - 1.0) < 0.1 && "Sample variance should be close to 1.0");
    
    // ========================================
    // PROPERTY 3: Different mean parameter works
    // ========================================
    unsigned int seed3 = 98765;
    double sum3 = 0;
    double target_mean = 5.0;
    int n3 = 5000;
    
    for (int i = 0; i < n3; i++) {
        double r = gaussian_box_muller(target_mean, 1.0, seed3, a, c, m);
        sum3 += r;
    }
    
    double mean3 = sum3 / n3;
    assert(std::abs(mean3 - target_mean) < 0.1 && "Mean should be close to 5.0");
    
    // ========================================
    // PROPERTY 4: Different sigma parameter works
    // ========================================
    unsigned int seed4 = 11111;
    double sum4 = 0;
    double sum_sq4 = 0;
    double target_sigma = 2.0;
    int n4 = 5000;
    
    for (int i = 0; i < n4; i++) {
        double r = gaussian_box_muller(0.0, target_sigma, seed4, a, c, m);
        sum4 += r;
        sum_sq4 += r * r;
    }
    
    double mean4 = sum4 / n4;
    double variance4 = (sum_sq4 / n4) - (mean4 * mean4);
    assert(std::abs(variance4 - target_sigma * target_sigma) < 0.3 && "Variance should be close to 4.0");
    
    // ========================================
    // PROPERTY 5: Deterministic behavior
    // ========================================
    unsigned int seed5a = 22222;
    unsigned int seed5b = 22222;
    double r1 = gaussian_box_muller(0.0, 1.0, seed5a, a, c, m);
    double r2 = gaussian_box_muller(0.0, 1.0, seed5b, a, c, m);
    assert(r1 == r2 && "Same seed should produce same value");
    
    // ========================================
    // PROPERTY 6: Different consecutive values
    // ========================================
    unsigned int seed6 = 33333;
    double prev = gaussian_box_muller(0.0, 1.0, seed6, a, c, m);
    int different_count = 0;
    for (int i = 0; i < 100; i++) {
        double curr = gaussian_box_muller(0.0, 1.0, seed6, a, c, m);
        if (std::abs(curr - prev) > 1e-10) different_count++;
        prev = curr;
    }
    assert(different_count > 90 && "Sequence should have mostly different values");
    
    // ========================================
    // PROPERTY 7: 68-95-99.7 rule (empirical rule)
    // ========================================
    // For standard normal: ~68% within 1σ, ~95% within 2σ, ~99.7% within 3σ
    unsigned int seed7 = 44444;
    int within_1sigma = 0, within_2sigma = 0, within_3sigma = 0;
    int n7 = 10000;
    
    for (int i = 0; i < n7; i++) {
        double r = gaussian_box_muller(0.0, 1.0, seed7, a, c, m);
        if (std::abs(r) <= 1.0) within_1sigma++;
        if (std::abs(r) <= 2.0) within_2sigma++;
        if (std::abs(r) <= 3.0) within_3sigma++;
    }
    
    double pct_1sigma = (double)within_1sigma / n7;
    double pct_2sigma = (double)within_2sigma / n7;
    double pct_3sigma = (double)within_3sigma / n7;
    
    assert(pct_1sigma > 0.60 && pct_1sigma < 0.75 && "~68% should be within 1 sigma");
    assert(pct_2sigma > 0.90 && pct_2sigma < 0.98 && "~95% should be within 2 sigma");
    assert(pct_3sigma > 0.95 && pct_3sigma < 1.00 && "~99.7% should be within 3 sigma");
    
    // ========================================
    // PROPERTY 8: Negative mean works
    // ========================================
    unsigned int seed8 = 55555;
    double sum8 = 0;
    double negative_mean = -3.0;
    int n8 = 5000;
    
    for (int i = 0; i < n8; i++) {
        double r = gaussian_box_muller(negative_mean, 1.0, seed8, a, c, m);
        sum8 += r;
    }
    
    double mean8 = sum8 / n8;
    assert(std::abs(mean8 - negative_mean) < 0.1 && "Negative mean should work");
    
    std::cout << "test_gaussian_box_muller PASSED" << std::endl;
    return 0;
}
