#include "../src/cppqed_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    const double tol = 1e-10;
    
    // Test 1: Simple state with kappa=1.0, nTh=0
    std::vector<dcomp> psi1 = {dcomp(0.0, 0.0), dcomp(1.0, 0.0), dcomp(0.0, 0.0)};
    auto result1 = apply_photon_loss_jump(psi1, 1.0, 0.0);
    double fact1 = std::sqrt(2.0 * 1.0 * 1.0);
    assert(std::abs(result1[0] - dcomp(fact1 * std::sqrt(1.0), 0.0)) < tol);
    assert(std::abs(result1[1]) < tol);
    assert(std::abs(result1[2]) < tol);
    
    // Test 2: Different parameters
    std::vector<dcomp> psi2 = {dcomp(1.0, 0.0), dcomp(2.0, 0.0), dcomp(3.0, 0.0), dcomp(0.0, 0.0)};
    auto result2 = apply_photon_loss_jump(psi2, 0.5, 0.5);
    double fact2 = std::sqrt(2.0 * 0.5 * 1.5);
    assert(std::abs(result2[0] - dcomp(fact2 * std::sqrt(1.0) * 2.0, 0.0)) < tol);
    assert(std::abs(result2[1] - dcomp(fact2 * std::sqrt(2.0) * 3.0, 0.0)) < tol);
    assert(std::abs(result2[2]) < tol);
    assert(std::abs(result2[3]) < tol); // Last element is zero
    
    // Test 3: Complex state
    std::vector<dcomp> psi3 = {dcomp(1.0, 1.0), dcomp(2.0, 2.0)};
    auto result3 = apply_photon_loss_jump(psi3, 1.0, 0.0);
    double fact3 = std::sqrt(2.0);
    assert(std::abs(result3[0] - dcomp(fact3 * std::sqrt(1.0) * 2.0, fact3 * std::sqrt(1.0) * 2.0)) < tol);
    assert(std::abs(result3[1]) < tol);
    
    // Test 4: Single element vector
    std::vector<dcomp> psi4 = {dcomp(5.0, 3.0)};
    auto result4 = apply_photon_loss_jump(psi4, 1.0, 0.0);
    assert(result4.size() == 1);
    assert(std::abs(result4[0]) < tol); // Last element is always zero
    
    // Test 5: kappa=0 (no decay)
    std::vector<dcomp> psi5 = {dcomp(1.0, 0.0), dcomp(2.0, 0.0), dcomp(3.0, 0.0)};
    auto result5 = apply_photon_loss_jump(psi5, 0.0, 0.0);
    for (const auto& val : result5) {
        assert(std::abs(val) < tol); // All zeros when kappa=0
    }
    
    // Test 6: Large nTh (thermal photons)
    std::vector<dcomp> psi6 = {dcomp(0.0, 0.0), dcomp(1.0, 0.0), dcomp(0.0, 0.0)};
    auto result6 = apply_photon_loss_jump(psi6, 1.0, 10.0);
    double fact6 = std::sqrt(2.0 * 1.0 * 11.0);
    assert(std::abs(result6[0] - dcomp(fact6, 0.0)) < tol);
    
    // Test 7: Verify formula: result[n] = fact * sqrt(n+1) * psi[n+1]
    std::vector<dcomp> psi7 = {dcomp(1.0, 0.0), dcomp(2.0, 0.0), dcomp(3.0, 0.0), 
                                dcomp(4.0, 0.0), dcomp(5.0, 0.0)};
    double kappa7 = 0.8;
    double nTh7 = 0.3;
    auto result7 = apply_photon_loss_jump(psi7, kappa7, nTh7);
    double fact7 = std::sqrt(2.0 * kappa7 * (nTh7 + 1.0));
    for (size_t n = 0; n < psi7.size() - 1; ++n) {
        dcomp expected = fact7 * std::sqrt(n + 1.0) * psi7[n + 1];
        assert(std::abs(result7[n] - expected) < tol);
    }
    assert(std::abs(result7[psi7.size() - 1]) < tol);
    
    // Test 8: Pure imaginary state
    std::vector<dcomp> psi8 = {dcomp(0.0, 1.0), dcomp(0.0, 2.0), dcomp(0.0, 3.0)};
    auto result8 = apply_photon_loss_jump(psi8, 1.0, 0.0);
    double fact8 = std::sqrt(2.0);
    assert(std::abs(result8[0] - dcomp(0.0, fact8 * std::sqrt(1.0) * 2.0)) < tol);
    assert(std::abs(result8[1] - dcomp(0.0, fact8 * std::sqrt(2.0) * 3.0)) < tol);
    
    // Test 9: Verify result size matches input size
    std::vector<dcomp> psi9 = {dcomp(1.0, 0.0), dcomp(2.0, 0.0), dcomp(3.0, 0.0), 
                                dcomp(4.0, 0.0), dcomp(5.0, 0.0), dcomp(6.0, 0.0)};
    auto result9 = apply_photon_loss_jump(psi9, 1.0, 0.0);
    assert(result9.size() == psi9.size());
    
    // Test 10: Zero state
    std::vector<dcomp> psi10 = {dcomp(0.0, 0.0), dcomp(0.0, 0.0), dcomp(0.0, 0.0)};
    auto result10 = apply_photon_loss_jump(psi10, 1.0, 0.0);
    for (const auto& val : result10) {
        assert(std::abs(val) < tol);
    }
    
    // Test 11: Verify sqrt(n+1) scaling
    std::vector<dcomp> psi11 = {dcomp(0.0, 0.0), dcomp(1.0, 0.0), dcomp(1.0, 0.0), 
                                 dcomp(1.0, 0.0), dcomp(1.0, 0.0)};
    auto result11 = apply_photon_loss_jump(psi11, 1.0, 0.0);
    double fact11 = std::sqrt(2.0);
    assert(std::abs(result11[0] - dcomp(fact11 * std::sqrt(1.0), 0.0)) < tol);
    assert(std::abs(result11[1] - dcomp(fact11 * std::sqrt(2.0), 0.0)) < tol);
    assert(std::abs(result11[2] - dcomp(fact11 * std::sqrt(3.0), 0.0)) < tol);
    assert(std::abs(result11[3] - dcomp(fact11 * std::sqrt(4.0), 0.0)) < tol);
    
    // Test 12: Different kappa and nTh combinations
    std::vector<dcomp> psi12 = {dcomp(1.0, 0.0), dcomp(1.0, 0.0)};
    auto result12a = apply_photon_loss_jump(psi12, 2.0, 0.0);
    auto result12b = apply_photon_loss_jump(psi12, 1.0, 1.0);
    // Both should give fact = sqrt(2*2*1) = 2 and fact = sqrt(2*1*2) = 2
    assert(std::abs(std::abs(result12a[0]) - std::abs(result12b[0])) < tol);
    
    // Test 13: Verify last element is always zero regardless of input
    std::vector<dcomp> psi13 = {dcomp(100.0, 50.0), dcomp(200.0, 100.0), dcomp(300.0, 150.0)};
    auto result13 = apply_photon_loss_jump(psi13, 5.0, 2.0);
    assert(std::abs(result13[result13.size() - 1]) < tol);
    
    std::cout << "All tests passed for apply_photon_loss_jump!" << std::endl;
    return 0;
}
