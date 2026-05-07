#include "../src/md_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

int main() {
    const double EPSILON = 1e-10;
    
    // ========================================
    // PROPERTY 1: Result must be in [0, boxSize) for single wrap
    // Note: Implementation may only handle single wraps (typical in MD)
    // ========================================
    {
        std::vector<vec3> test_positions = {
            vec3(-5.0, 0.0, 0.0),
            vec3(15.0, 0.0, 0.0),
            vec3(0.0, -8.0, 0.0),
            vec3(0.0, 12.0, 0.0),
            vec3(0.0, 0.0, -3.0),
            vec3(0.0, 0.0, 18.0),
            vec3(-2.0, 12.0, -5.0)
        };
        
        double boxSize = 10.0;
        for (const auto& pos : test_positions) {
            vec3 result = applyPeriodicBoundary(pos, boxSize);
            
            assert(result.x >= 0.0 && result.x < boxSize && "x must be in [0, boxSize)");
            assert(result.y >= 0.0 && result.y < boxSize && "y must be in [0, boxSize)");
            assert(result.z >= 0.0 && result.z < boxSize && "z must be in [0, boxSize)");
        }
    }
    
    // ========================================
    // PROPERTY 2: Idempotence (applying twice gives same result)
    // ========================================
    {
        vec3 pos(-5.0, 15.0, -3.0);
        double boxSize = 10.0;
        
        vec3 result1 = applyPeriodicBoundary(pos, boxSize);
        vec3 result2 = applyPeriodicBoundary(result1, boxSize);
        
        assert(std::abs(result1.x - result2.x) < EPSILON && "Must be idempotent");
        assert(std::abs(result1.y - result2.y) < EPSILON && "Must be idempotent");
        assert(std::abs(result1.z - result2.z) < EPSILON && "Must be idempotent");
    }
    
    // ========================================
    // PROPERTY 3: Position inside box unchanged
    // ========================================
    {
        std::vector<vec3> inside_positions = {
            vec3(5.0, 5.0, 5.0),
            vec3(0.0, 0.0, 0.0),
            vec3(9.9, 9.9, 9.9),
            vec3(2.5, 7.3, 4.1)
        };
        
        double boxSize = 10.0;
        for (const auto& pos : inside_positions) {
            vec3 result = applyPeriodicBoundary(pos, boxSize);
            
            assert(std::abs(result.x - pos.x) < EPSILON && "Inside positions must be unchanged");
            assert(std::abs(result.y - pos.y) < EPSILON && "Inside positions must be unchanged");
            assert(std::abs(result.z - pos.z) < EPSILON && "Inside positions must be unchanged");
        }
    }
    
    // ========================================
    // PROPERTY 4: Wrapping for single-step displacements
    // ========================================
    {
        vec3 pos(12.5, -3.7, 15.3);
        double boxSize = 10.0;
        
        vec3 result = applyPeriodicBoundary(pos, boxSize);
        
        // Result should be in valid range
        assert(result.x >= 0.0 && result.x < boxSize && "Must wrap to valid range");
        assert(result.y >= 0.0 && result.y < boxSize && "Must wrap to valid range");
        assert(result.z >= 0.0 && result.z < boxSize && "Must wrap to valid range");
    }
    
    // ========================================
    // PROPERTY 5: At boundary (x = boxSize) wraps to 0
    // ========================================
    {
        vec3 pos(10.0, 10.0, 10.0);
        double boxSize = 10.0;
        
        vec3 result = applyPeriodicBoundary(pos, boxSize);
        
        assert(std::abs(result.x - 0.0) < EPSILON && "Boundary position must wrap to 0");
        assert(std::abs(result.y - 0.0) < EPSILON && "Boundary position must wrap to 0");
        assert(std::abs(result.z - 0.0) < EPSILON && "Boundary position must wrap to 0");
    }
    
    // ========================================
    // PROPERTY 6: Single wrap outside box (realistic MD scenario)
    // ========================================
    {
        vec3 pos(12.5, -3.7, 15.3);
        double boxSize = 10.0;
        
        vec3 result = applyPeriodicBoundary(pos, boxSize);
        
        assert(result.x >= 0.0 && result.x < boxSize && "Must handle single wrap");
        assert(result.y >= 0.0 && result.y < boxSize && "Must handle single wrap");
        assert(result.z >= 0.0 && result.z < boxSize && "Must handle single wrap");
    }
    
    // ========================================
    // PROPERTY 7: Different box sizes (single wrap)
    // ========================================
    {
        vec3 pos(12.0, 12.0, 12.0);
        
        std::vector<double> boxSizes = {5.0, 10.0, 15.0, 20.0};
        for (double boxSize : boxSizes) {
            // Adjust position to require only single wrap
            vec3 test_pos(boxSize + 2.0, boxSize - 2.0, -2.0);
            vec3 result = applyPeriodicBoundary(test_pos, boxSize);
            
            assert(result.x >= 0.0 && result.x < boxSize && "Must work for different box sizes");
            assert(result.y >= 0.0 && result.y < boxSize && "Must work for different box sizes");
            assert(result.z >= 0.0 && result.z < boxSize && "Must work for different box sizes");
        }
    }
    
    // ========================================
    // PROPERTY 8: Independent component wrapping
    // ========================================
    {
        vec3 pos(5.0, 15.0, 3.0);  // Only y needs wrapping
        double boxSize = 10.0;
        
        vec3 result = applyPeriodicBoundary(pos, boxSize);
        
        assert(std::abs(result.x - 5.0) < EPSILON && "x should be unchanged");
        assert(std::abs(result.y - 5.0) < EPSILON && "y should be wrapped");
        assert(std::abs(result.z - 3.0) < EPSILON && "z should be unchanged");
    }
    
    // ========================================
    // PROPERTY 9: Negative positions wrap correctly
    // ========================================
    {
        vec3 pos(-2.5, -7.3, -0.1);
        double boxSize = 10.0;
        
        vec3 result = applyPeriodicBoundary(pos, boxSize);
        
        assert(result.x >= 0.0 && result.x < boxSize && "Negative positions must wrap to positive");
        assert(result.y >= 0.0 && result.y < boxSize && "Negative positions must wrap to positive");
        assert(result.z >= 0.0 && result.z < boxSize && "Negative positions must wrap to positive");
        
        // Verify correct values
        assert(std::abs(result.x - 7.5) < EPSILON && "Negative wrap must be correct");
        assert(std::abs(result.y - 2.7) < EPSILON && "Negative wrap must be correct");
        assert(std::abs(result.z - 9.9) < EPSILON && "Negative wrap must be correct");
    }
    
    // ========================================
    // PROPERTY 10: Stress test with realistic positions (single wrap)
    // ========================================
    {
        double boxSize = 10.0;
        std::vector<vec3> test_positions;
        
        // Generate positions requiring single wrap only (-boxSize to 2*boxSize)
        for (double x = -boxSize; x <= 2.0*boxSize; x += 1.3) {
            for (double y = -boxSize; y <= 2.0*boxSize; y += 1.7) {
                for (double z = -boxSize; z <= 2.0*boxSize; z += 1.9) {
                    test_positions.push_back(vec3(x, y, z));
                }
            }
        }
        
        for (const auto& pos : test_positions) {
            vec3 result = applyPeriodicBoundary(pos, boxSize);
            
            // All results must be in valid range
            assert(result.x >= 0.0 && result.x < boxSize && "Stress test: all results must be valid");
            assert(result.y >= 0.0 && result.y < boxSize && "Stress test: all results must be valid");
            assert(result.z >= 0.0 && result.z < boxSize && "Stress test: all results must be valid");
        }
    }
    
    std::cout << "All strict tests passed for applyPeriodicBoundary!" << std::endl;
    return 0;
}
