#include "../src/cppqed_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    const double tol = 1e-9;
    
    // Test 1: t=0, gamma=0.5, omega=1.0
    // c(0) = exp(0) / (1 + 2i*0.5*1.0 - 1^2) = 1 / (1 + i - 1) = 1/i = -i
    dcomp result1 = calculate_ddho_particular_solution(0.0, 0.5, 1.0);
    dcomp expected1 = dcomp(0.0, -1.0);
    assert(std::abs(result1 - expected1) < tol);
    
    // Test 2: t=1.0, gamma=0.5, omega=2.0
    // c(1) = exp(2i) / (1 + 2i*0.5*2.0 - 4) = exp(2i) / (1 + 2i - 4) = exp(2i) / (-3 + 2i)
    dcomp result2 = calculate_ddho_particular_solution(1.0, 0.5, 2.0);
    dcomp numerator = std::exp(1i * 2.0);
    dcomp denominator = dcomp(-3.0, 2.0);
    dcomp expected2 = numerator / denominator;
    assert(std::abs(result2 - expected2) < tol);
    
    // Test 3: t=0.5, gamma=1.0, omega=1.5
    // c(0.5) = exp(1.5i*0.5) / (1 + 2i*1.0*1.5 - 1.5^2)
    //        = exp(0.75i) / (1 + 3i - 2.25) = exp(0.75i) / (-1.25 + 3i)
    dcomp result3 = calculate_ddho_particular_solution(0.5, 1.0, 1.5);
    dcomp num3 = std::exp(1i * 0.75);
    dcomp den3 = dcomp(-1.25, 3.0);
    dcomp expected3 = num3 / den3;
    assert(std::abs(result3 - expected3) < tol);
    
    // Test 4: Undamped case gamma=0, omega=0.5
    // c(t) = exp(0.5i*t) / (1 - 0.25) = exp(0.5i*t) / 0.75
    dcomp result4 = calculate_ddho_particular_solution(2.0, 0.0, 0.5);
    dcomp expected4 = std::exp(1i * 1.0) / 0.75;
    assert(std::abs(result4 - expected4) < tol);
    
    // Test 5: No driving omega=0
    // c(t) = exp(0) / 1 = 1
    dcomp result5 = calculate_ddho_particular_solution(1.0, 0.5, 0.0);
    dcomp expected5 = dcomp(1.0, 0.0);
    assert(std::abs(result5 - expected5) < tol);
    
    // Test 6: Negative time - verify result is finite
    dcomp result6 = calculate_ddho_particular_solution(-1.0, 0.5, 1.0);
    assert(std::isfinite(std::abs(result6)));
    
    // Test 7: Resonance condition omega=1, gamma=0
    // Denominator: 1 - 1 = 0, but with small gamma to avoid singularity
    dcomp result7 = calculate_ddho_particular_solution(0.0, 0.01, 1.0);
    dcomp expected7 = dcomp(1.0, 0.0) / dcomp(0.0, 0.02);  // 1/(0.02i) = -50i
    assert(std::abs(result7 - expected7) < 1e-6);  // Slightly larger tolerance for near-singularity
    
    // Test 8: Verify result is always finite
    dcomp result8 = calculate_ddho_particular_solution(3.14, 2.0, 0.8);
    assert(std::isfinite(std::real(result8)));
    assert(std::isfinite(std::imag(result8)));
    
    // Test 9: Large gamma (overdamped)
    dcomp result9 = calculate_ddho_particular_solution(1.0, 10.0, 0.5);
    dcomp expected9 = std::exp(0.5i) / (1.0 + 10.0i - 0.25);
    assert(std::abs(result9 - expected9) < tol);
    
    // Test 10: Verify time dependence - phase should increase with time
    double t1 = 0.0, t2 = 1.0;
    dcomp c1 = calculate_ddho_particular_solution(t1, 0.5, 1.0);
    dcomp c2 = calculate_ddho_particular_solution(t2, 0.5, 1.0);
    double phase1 = std::arg(c1);
    double phase2 = std::arg(c2);
    // Phase should increase by omega*dt = 1.0*1.0 = 1.0 radian
    assert(std::abs((phase2 - phase1) - 1.0) < tol || 
           std::abs((phase2 - phase1) - 1.0 + 2*PI) < tol ||
           std::abs((phase2 - phase1) - 1.0 - 2*PI) < tol);
    
    std::cout << "All tests passed for calculate_ddho_particular_solution!" << std::endl;
    return 0;
}
