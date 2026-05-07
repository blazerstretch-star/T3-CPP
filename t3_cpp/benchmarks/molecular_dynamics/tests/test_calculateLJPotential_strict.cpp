#include "../src/md_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

int main() {
    const double EPSILON = 1e-6;
    
    // ========================================
    // PROPERTY 1: V(r=sigma) = 0 (zero crossing)
    // ========================================
    {
        vec3 r(1.0, 0.0, 0.0);
        double pot = calculateLJPotential(r, 1.0, 1.0);
        assert(std::abs(pot - 0.0) < EPSILON && "LJ potential at r=sigma must be zero");
    }
    
    // ========================================
    // PROPERTY 2: V(r=2^(1/6)*sigma) = -epsilon (minimum)
    // ========================================
    {
        double r_min = std::pow(2.0, 1.0/6.0);
        vec3 r(r_min, 0.0, 0.0);
        double pot = calculateLJPotential(r, 1.0, 1.0);
        assert(std::abs(pot + 1.0) < EPSILON && "LJ potential minimum must be -epsilon");
    }
    
    // ========================================
    // PROPERTY 3: V(r) > 0 for r < sigma (repulsive)
    // ========================================
    {
        vec3 r(0.8, 0.0, 0.0);
        double pot = calculateLJPotential(r, 1.0, 1.0);
        assert(pot > 0.0 && "LJ potential must be positive (repulsive) for r < sigma");
    }
    
    // ========================================
    // PROPERTY 4: V(r) < 0 for sigma < r < 2^(1/6)*sigma (attractive)
    // ========================================
    {
        vec3 r(1.05, 0.0, 0.0);
        double pot = calculateLJPotential(r, 1.0, 1.0);
        assert(pot < 0.0 && "LJ potential must be negative (attractive) for r slightly > sigma");
    }
    
    // ========================================
    // PROPERTY 5: V(r) approaches 0 as r -> infinity
    // ========================================
    {
        vec3 r(100.0, 0.0, 0.0);
        double pot = calculateLJPotential(r, 1.0, 1.0);
        assert(std::abs(pot) < 1e-10 && "LJ potential must approach zero at large distances");
    }
    
    // ========================================
    // PROPERTY 6: Direction independence (spherical symmetry)
    // ========================================
    {
        double dist = 1.5;
        vec3 r1(dist, 0.0, 0.0);
        vec3 r2(0.0, dist, 0.0);
        vec3 r3(0.0, 0.0, dist);
        vec3 r4(dist/std::sqrt(3.0), dist/std::sqrt(3.0), dist/std::sqrt(3.0));
        
        double pot1 = calculateLJPotential(r1, 1.0, 1.0);
        double pot2 = calculateLJPotential(r2, 1.0, 1.0);
        double pot3 = calculateLJPotential(r3, 1.0, 1.0);
        double pot4 = calculateLJPotential(r4, 1.0, 1.0);
        
        assert(std::abs(pot1 - pot2) < EPSILON && "LJ potential must be spherically symmetric");
        assert(std::abs(pot1 - pot3) < EPSILON && "LJ potential must be spherically symmetric");
        assert(std::abs(pot1 - pot4) < EPSILON && "LJ potential must be spherically symmetric");
    }
    
    // ========================================
    // PROPERTY 7: Scaling with epsilon
    // ========================================
    {
        vec3 r(1.5, 0.0, 0.0);
        double pot1 = calculateLJPotential(r, 1.0, 1.0);
        double pot2 = calculateLJPotential(r, 2.0, 1.0);
        assert(std::abs(pot2 - 2.0 * pot1) < EPSILON && "LJ potential must scale linearly with epsilon");
    }
    
    // ========================================
    // PROPERTY 8: Scaling with sigma
    // ========================================
    {
        vec3 r1(2.0, 0.0, 0.0);
        vec3 r2(4.0, 0.0, 0.0);
        double pot1 = calculateLJPotential(r1, 1.0, 1.0);
        double pot2 = calculateLJPotential(r2, 1.0, 2.0);
        assert(std::abs(pot1 - pot2) < EPSILON && "LJ potential must scale with sigma/r ratio");
    }
    
    // ========================================
    // PROPERTY 9: Correct formula V = 4*epsilon*((sigma/r)^12 - (sigma/r)^6)
    // ========================================
    {
        vec3 r(2.0, 0.0, 0.0);
        double pot = calculateLJPotential(r, 1.5, 1.2);
        
        double dist = r.length();
        double sigma_over_r = 1.2 / dist;
        double term6 = std::pow(sigma_over_r, 6);
        double term12 = term6 * term6;
        double expected = 4.0 * 1.5 * (term12 - term6);
        
        assert(std::abs(pot - expected) < EPSILON && "LJ potential must follow correct formula");
    }
    
    // ========================================
    // PROPERTY 10: Zero distance handling (avoid division by zero)
    // ========================================
    {
        vec3 r(0.0, 0.0, 0.0);
        double pot = calculateLJPotential(r, 1.0, 1.0);
        // Should return 0 or very large positive value, not crash
        assert(!std::isnan(pot) && !std::isinf(pot) && "Must handle zero distance gracefully");
    }
    
    // ========================================
    // PROPERTY 11: Very small distance (numerical stability)
    // ========================================
    {
        vec3 r(1e-12, 0.0, 0.0);
        double pot = calculateLJPotential(r, 1.0, 1.0);
        assert(pot >= 0.0 && "Very small distances should give large positive (repulsive) potential");
    }
    
    // ========================================
    // PROPERTY 12: Multiple parameter combinations
    // ========================================
    {
        std::vector<double> epsilons = {0.5, 1.0, 2.0, 5.0};
        std::vector<double> sigmas = {0.8, 1.0, 1.5, 2.0};
        std::vector<double> distances = {0.9, 1.0, 1.2, 2.0, 5.0};
        
        for (double eps : epsilons) {
            for (double sig : sigmas) {
                for (double d : distances) {
                    vec3 r(d, 0.0, 0.0);
                    double pot = calculateLJPotential(r, eps, sig);
                    
                    // Verify formula
                    double sigma_over_r = sig / d;
                    double term6 = std::pow(sigma_over_r, 6);
                    double term12 = term6 * term6;
                    double expected = 4.0 * eps * (term12 - term6);
                    
                    assert(std::abs(pot - expected) < EPSILON && "Formula must hold for all parameter combinations");
                }
            }
        }
    }
    
    std::cout << "All strict tests passed for calculateLJPotential!" << std::endl;
    return 0;
}
