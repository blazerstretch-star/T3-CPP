#include "../src/md_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

int main() {
    const double EPSILON = 1e-6;
    const double r_min = std::pow(2.0, 1.0/6.0);
    
    // ========================================
    // PROPERTY 1: Force at r_min should be zero (equilibrium)
    // ========================================
    {
        vec3 r(r_min, 0.0, 0.0);
        vec3 force = calculateLJForce(r, 1.0, 1.0);
        assert(std::abs(force.x) < EPSILON && "Force at r_min must be zero");
        assert(std::abs(force.y) < EPSILON && "Force at r_min must be zero");
        assert(std::abs(force.z) < EPSILON && "Force at r_min must be zero");
    }
    
    // ========================================
    // PROPERTY 2: Force points along separation vector
    // ========================================
    {
        vec3 r(1.5, 1.5, 0.0);
        vec3 force = calculateLJForce(r, 1.0, 1.0);
        
        // Force should be parallel to r: F = k*r for some scalar k
        double ratio_xy = force.y / force.x;
        double expected_ratio = r.y / r.x;
        assert(std::abs(ratio_xy - expected_ratio) < EPSILON && "Force must be parallel to separation vector");
    }
    
    // ========================================
    // PROPERTY 3: Repulsive force for r < r_min (points away)
    // ========================================
    {
        vec3 r(0.9, 0.0, 0.0);
        vec3 force = calculateLJForce(r, 1.0, 1.0);
        
        // Force should point in same direction as r (repulsive)
        double dot_product = force.x * r.x + force.y * r.y + force.z * r.z;
        assert(dot_product > 0.0 && "Repulsive force must point away from origin");
    }
    
    // ========================================
    // PROPERTY 4: Attractive force for r > r_min (points toward)
    // ========================================
    {
        vec3 r(2.0, 0.0, 0.0);
        vec3 force = calculateLJForce(r, 1.0, 1.0);
        
        // Force should point opposite to r (attractive)
        double dot_product = force.x * r.x + force.y * r.y + force.z * r.z;
        assert(dot_product < 0.0 && "Attractive force must point toward origin");
    }
    
    // ========================================
    // PROPERTY 5: Force magnitude approaches zero at large distances
    // ========================================
    {
        vec3 r(100.0, 0.0, 0.0);
        vec3 force = calculateLJForce(r, 1.0, 1.0);
        double magnitude = force.length();
        assert(magnitude < 1e-10 && "Force must approach zero at large distances");
    }
    
    // ========================================
    // PROPERTY 6: Spherical symmetry (direction independence)
    // ========================================
    {
        double dist = 1.5;
        vec3 r1(dist, 0.0, 0.0);
        vec3 r2(0.0, dist, 0.0);
        vec3 r3(0.0, 0.0, dist);
        
        vec3 f1 = calculateLJForce(r1, 1.0, 1.0);
        vec3 f2 = calculateLJForce(r2, 1.0, 1.0);
        vec3 f3 = calculateLJForce(r3, 1.0, 1.0);
        
        double mag1 = f1.length();
        double mag2 = f2.length();
        double mag3 = f3.length();
        
        assert(std::abs(mag1 - mag2) < EPSILON && "Force magnitude must be spherically symmetric");
        assert(std::abs(mag1 - mag3) < EPSILON && "Force magnitude must be spherically symmetric");
    }
    
    // ========================================
    // PROPERTY 7: Force scales linearly with epsilon
    // ========================================
    {
        vec3 r(1.5, 0.0, 0.0);
        vec3 f1 = calculateLJForce(r, 1.0, 1.0);
        vec3 f2 = calculateLJForce(r, 2.0, 1.0);
        
        assert(std::abs(f2.x - 2.0 * f1.x) < EPSILON && "Force must scale linearly with epsilon");
    }
    
    // ========================================
    // PROPERTY 8: Correct formula F = 24*epsilon*(2*(sigma/r)^12 - (sigma/r)^6) * r/r^2
    // ========================================
    {
        vec3 r(2.0, 0.0, 0.0);
        vec3 force = calculateLJForce(r, 1.5, 1.2);
        
        double dist = r.length();
        double sigma_over_r = 1.2 / dist;
        double term6 = std::pow(sigma_over_r, 6);
        double term12 = term6 * term6;
        double force_magnitude = 24.0 * 1.5 * (2.0 * term12 - term6) / dist;
        vec3 expected = r * (force_magnitude / dist);
        
        assert(std::abs(force.x - expected.x) < EPSILON && "Force must follow correct formula");
    }
    
    // ========================================
    // PROPERTY 9: Force is negative gradient of potential
    // ========================================
    {
        vec3 r(1.5, 0.0, 0.0);
        double h = 1e-6;
        
        vec3 r_plus(r.x + h, r.y, r.z);
        vec3 r_minus(r.x - h, r.y, r.z);
        
        double V_plus = calculateLJPotential(r_plus, 1.0, 1.0);
        double V_minus = calculateLJPotential(r_minus, 1.0, 1.0);
        
        double numerical_force_x = -(V_plus - V_minus) / (2.0 * h);
        
        vec3 force = calculateLJForce(r, 1.0, 1.0);
        
        assert(std::abs(force.x - numerical_force_x) < 1e-4 && "Force must be negative gradient of potential");
    }
    
    // ========================================
    // PROPERTY 10: Zero distance handling
    // ========================================
    {
        vec3 r(0.0, 0.0, 0.0);
        vec3 force = calculateLJForce(r, 1.0, 1.0);
        
        assert(!std::isnan(force.x) && !std::isinf(force.x) && "Must handle zero distance gracefully");
        assert(force.x == 0.0 && force.y == 0.0 && force.z == 0.0 && "Zero distance should give zero force");
    }
    
    // ========================================
    // PROPERTY 11: Very small distance (numerical stability)
    // ========================================
    {
        vec3 r(1e-12, 0.0, 0.0);
        vec3 force = calculateLJForce(r, 1.0, 1.0);
        
        // Should give large repulsive force or zero, not NaN
        assert(!std::isnan(force.x) && !std::isinf(force.x) && "Must be numerically stable at small distances");
    }
    
    // ========================================
    // PROPERTY 12: Force in 3D (all components)
    // ========================================
    {
        vec3 r(1.0, 1.0, 1.0);
        vec3 force = calculateLJForce(r, 1.0, 1.0);
        
        // All components should be equal due to symmetry
        assert(std::abs(force.x - force.y) < EPSILON && "Force components must respect symmetry");
        assert(std::abs(force.x - force.z) < EPSILON && "Force components must respect symmetry");
        
        // Force should be parallel to r
        double ratio_xy = force.y / force.x;
        double expected_ratio = r.y / r.x;
        assert(std::abs(ratio_xy - expected_ratio) < EPSILON && "Force must be parallel to r in 3D");
    }
    
    // ========================================
    // PROPERTY 13: Multiple parameter combinations
    // ========================================
    {
        std::vector<double> epsilons = {0.5, 1.0, 2.0};
        std::vector<double> sigmas = {0.8, 1.0, 1.5};
        std::vector<double> distances = {0.9, 1.2, 2.0};
        
        for (double eps : epsilons) {
            for (double sig : sigmas) {
                for (double d : distances) {
                    vec3 r(d, 0.0, 0.0);
                    vec3 force = calculateLJForce(r, eps, sig);
                    
                    // Verify formula
                    double sigma_over_r = sig / d;
                    double term6 = std::pow(sigma_over_r, 6);
                    double term12 = term6 * term6;
                    double force_magnitude = 24.0 * eps * (2.0 * term12 - term6) / d;
                    vec3 expected = r * (force_magnitude / d);
                    
                    assert(std::abs(force.x - expected.x) < EPSILON && "Formula must hold for all parameters");
                }
            }
        }
    }
    
    std::cout << "All strict tests passed for calculateLJForce!" << std::endl;
    return 0;
}
