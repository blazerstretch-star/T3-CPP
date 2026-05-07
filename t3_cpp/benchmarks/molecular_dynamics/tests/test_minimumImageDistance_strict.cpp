#include "../src/md_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

int main() {
    const double EPSILON = 1e-10;
    
    // ========================================
    // PROPERTY 1: Antisymmetry d(r1,r2) = -d(r2,r1)
    // ========================================
    {
        vec3 r1(2.0, 3.0, 4.0);
        vec3 r2(7.0, 8.0, 9.0);
        double boxSize = 10.0;
        
        vec3 dr12 = minimumImageDistance(r1, r2, boxSize);
        vec3 dr21 = minimumImageDistance(r2, r1, boxSize);
        
        assert(std::abs(dr12.x + dr21.x) < EPSILON && "Must be antisymmetric");
        assert(std::abs(dr12.y + dr21.y) < EPSILON && "Must be antisymmetric");
        assert(std::abs(dr12.z + dr21.z) < EPSILON && "Must be antisymmetric");
    }
    
    // ========================================
    // PROPERTY 2: Distance magnitude <= boxSize/2
    // ========================================
    {
        std::vector<vec3> positions = {
            vec3(0.0, 0.0, 0.0),
            vec3(9.0, 9.0, 9.0),
            vec3(1.0, 5.0, 8.0),
            vec3(2.5, 7.5, 3.5)
        };
        
        double boxSize = 10.0;
        for (size_t i = 0; i < positions.size(); ++i) {
            for (size_t j = i + 1; j < positions.size(); ++j) {
                vec3 dr = minimumImageDistance(positions[i], positions[j], boxSize);
                
                assert(std::abs(dr.x) <= boxSize / 2.0 + EPSILON && "Each component must be <= boxSize/2");
                assert(std::abs(dr.y) <= boxSize / 2.0 + EPSILON && "Each component must be <= boxSize/2");
                assert(std::abs(dr.z) <= boxSize / 2.0 + EPSILON && "Each component must be <= boxSize/2");
            }
        }
    }
    
    // ========================================
    // PROPERTY 3: No wrapping when distance < boxSize/2
    // ========================================
    {
        vec3 r1(2.0, 2.0, 2.0);
        vec3 r2(5.0, 5.0, 5.0);
        double boxSize = 10.0;
        
        vec3 dr = minimumImageDistance(r1, r2, boxSize);
        vec3 direct = r2 - r1;
        
        assert(std::abs(dr.x - direct.x) < EPSILON && "No wrapping needed for close particles");
        assert(std::abs(dr.y - direct.y) < EPSILON && "No wrapping needed for close particles");
        assert(std::abs(dr.z - direct.z) < EPSILON && "No wrapping needed for close particles");
    }
    
    // ========================================
    // PROPERTY 4: Wrapping across boundary
    // ========================================
    {
        vec3 r1(1.0, 0.0, 0.0);
        vec3 r2(9.0, 0.0, 0.0);
        double boxSize = 10.0;
        
        vec3 dr = minimumImageDistance(r1, r2, boxSize);
        
        // Minimum distance is -2.0 (wrapping around)
        assert(std::abs(dr.x - (-2.0)) < EPSILON && "Must wrap across boundary");
        assert(std::abs(dr.y - 0.0) < EPSILON && "Other components unchanged");
        assert(std::abs(dr.z - 0.0) < EPSILON && "Other components unchanged");
    }
    
    // ========================================
    // PROPERTY 5: Exactly at half box size
    // ========================================
    {
        vec3 r1(0.0, 0.0, 0.0);
        vec3 r2(5.0, 0.0, 0.0);
        double boxSize = 10.0;
        
        vec3 dr = minimumImageDistance(r1, r2, boxSize);
        
        // At exactly boxSize/2, either +5 or -5 is valid
        assert((std::abs(dr.x - 5.0) < EPSILON || std::abs(dr.x + 5.0) < EPSILON) && 
               "Half box distance can be either direction");
    }
    
    // ========================================
    // PROPERTY 6: Multiple periodic boundaries
    // ========================================
    {
        vec3 r1(1.0, 1.0, 1.0);
        vec3 r2(9.0, 9.0, 9.0);
        double boxSize = 10.0;
        
        vec3 dr = minimumImageDistance(r1, r2, boxSize);
        
        // All components should wrap
        assert(std::abs(dr.x - (-2.0)) < EPSILON && "x must wrap");
        assert(std::abs(dr.y - (-2.0)) < EPSILON && "y must wrap");
        assert(std::abs(dr.z - (-2.0)) < EPSILON && "z must wrap");
    }
    
    // ========================================
    // PROPERTY 7: Independent component wrapping
    // ========================================
    {
        vec3 r1(1.0, 5.0, 3.0);
        vec3 r2(9.0, 6.0, 4.0);
        double boxSize = 10.0;
        
        vec3 dr = minimumImageDistance(r1, r2, boxSize);
        
        // x wraps, y and z don't
        assert(std::abs(dr.x - (-2.0)) < EPSILON && "x must wrap");
        assert(std::abs(dr.y - 1.0) < EPSILON && "y no wrap");
        assert(std::abs(dr.z - 1.0) < EPSILON && "z no wrap");
    }
    
    // ========================================
    // PROPERTY 8: Self-distance is zero
    // ========================================
    {
        vec3 r(3.5, 7.2, 1.8);
        double boxSize = 10.0;
        
        vec3 dr = minimumImageDistance(r, r, boxSize);
        
        assert(std::abs(dr.x) < EPSILON && "Self-distance must be zero");
        assert(std::abs(dr.y) < EPSILON && "Self-distance must be zero");
        assert(std::abs(dr.z) < EPSILON && "Self-distance must be zero");
    }
    
    // ========================================
    // PROPERTY 9: Different box sizes (with appropriate particle positions)
    // ========================================
    {
        std::vector<double> boxSizes = {10.0, 15.0, 20.0};
        for (double boxSize : boxSizes) {
            // Use particles within the box
            vec3 r1(boxSize * 0.1, boxSize * 0.1, boxSize * 0.1);
            vec3 r2(boxSize * 0.9, boxSize * 0.9, boxSize * 0.9);
            
            vec3 dr = minimumImageDistance(r1, r2, boxSize);
            
            // Each component must be <= boxSize/2
            assert(std::abs(dr.x) <= boxSize / 2.0 + EPSILON && "Must respect box size");
            assert(std::abs(dr.y) <= boxSize / 2.0 + EPSILON && "Must respect box size");
            assert(std::abs(dr.z) <= boxSize / 2.0 + EPSILON && "Must respect box size");
        }
    }
    
    // ========================================
    // PROPERTY 10: Triangle inequality with wrapping
    // ========================================
    {
        vec3 r1(1.0, 1.0, 1.0);
        vec3 r2(5.0, 5.0, 5.0);
        vec3 r3(9.0, 9.0, 9.0);
        double boxSize = 10.0;
        
        vec3 dr12 = minimumImageDistance(r1, r2, boxSize);
        vec3 dr23 = minimumImageDistance(r2, r3, boxSize);
        vec3 dr13 = minimumImageDistance(r1, r3, boxSize);
        
        double dist12 = dr12.length();
        double dist23 = dr23.length();
        double dist13 = dr13.length();
        
        // Triangle inequality (with some tolerance for wrapping)
        assert(dist13 <= dist12 + dist23 + EPSILON && "Triangle inequality must hold");
    }
    
    // ========================================
    // PROPERTY 11: Consistency with direct distance for nearby particles
    // ========================================
    {
        // Test many pairs of nearby particles
        double boxSize = 10.0;
        for (double x1 = 2.0; x1 <= 8.0; x1 += 2.0) {
            for (double x2 = x1; x2 <= x1 + 3.0; x2 += 1.0) {
                vec3 r1(x1, 5.0, 5.0);
                vec3 r2(x2, 5.0, 5.0);
                
                vec3 dr_min = minimumImageDistance(r1, r2, boxSize);
                vec3 dr_direct = r2 - r1;
                
                // For nearby particles, minimum image should equal direct distance
                if (std::abs(dr_direct.x) < boxSize / 2.0) {
                    assert(std::abs(dr_min.x - dr_direct.x) < EPSILON && 
                           "Minimum image must equal direct distance for nearby particles");
                }
            }
        }
    }
    
    // ========================================
    // PROPERTY 12: Stress test with many positions
    // ========================================
    {
        double boxSize = 10.0;
        std::vector<vec3> positions;
        
        for (double x = 0.5; x < boxSize; x += 1.7) {
            for (double y = 0.5; y < boxSize; y += 1.9) {
                for (double z = 0.5; z < boxSize; z += 2.1) {
                    positions.push_back(vec3(x, y, z));
                }
            }
        }
        
        for (size_t i = 0; i < positions.size(); ++i) {
            for (size_t j = i + 1; j < positions.size(); ++j) {
                vec3 dr = minimumImageDistance(positions[i], positions[j], boxSize);
                
                // All components must be <= boxSize/2
                assert(std::abs(dr.x) <= boxSize / 2.0 + EPSILON && "Stress test: valid range");
                assert(std::abs(dr.y) <= boxSize / 2.0 + EPSILON && "Stress test: valid range");
                assert(std::abs(dr.z) <= boxSize / 2.0 + EPSILON && "Stress test: valid range");
                
                // Antisymmetry
                vec3 dr_rev = minimumImageDistance(positions[j], positions[i], boxSize);
                assert(std::abs(dr.x + dr_rev.x) < EPSILON && "Stress test: antisymmetry");
            }
        }
    }
    
    std::cout << "All strict tests passed for minimumImageDistance!" << std::endl;
    return 0;
}
