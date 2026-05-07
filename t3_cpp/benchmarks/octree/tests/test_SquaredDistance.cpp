#include "../src/octree_functions.h"
#include <cassert>
#include <cmath>

using namespace OrthoTree;

int main() {
    // Test 1: Distance between origin and (1,1,1)
    Point3D p1{0.0, 0.0, 0.0};
    Point3D p2{1.0, 1.0, 1.0};
    assert(std::abs(SquaredDistance(p1, p2) - 3.0) < 1e-9);
    
    // Test 2: Distance between identical points (zero)
    Point3D p3{5.0, 5.0, 5.0};
    Point3D p4{5.0, 5.0, 5.0};
    assert(std::abs(SquaredDistance(p3, p4)) < 1e-9);
    
    // Test 3: Distance along single axis
    Point3D p5{0.0, 0.0, 0.0};
    Point3D p6{3.0, 0.0, 0.0};
    assert(std::abs(SquaredDistance(p5, p6) - 9.0) < 1e-9);
    
    // Test 4: Negative coordinates
    Point3D p7{-1.0, -1.0, -1.0};
    Point3D p8{1.0, 1.0, 1.0};
    assert(std::abs(SquaredDistance(p7, p8) - 12.0) < 1e-9);
    
    // Test 5: 3-4-5 triangle (squared: 9+16=25)
    Point3D p9{0.0, 0.0, 0.0};
    Point3D p10{3.0, 4.0, 0.0};
    assert(std::abs(SquaredDistance(p9, p10) - 25.0) < 1e-9);
    
    // Test 6: Large distance
    Point3D p11{0.0, 0.0, 0.0};
    Point3D p12{100.0, 100.0, 100.0};
    assert(std::abs(SquaredDistance(p11, p12) - 30000.0) < 1e-9);
    
    // Test 7: Symmetry test (distance should be same both ways)
    Point3D p13{1.0, 2.0, 3.0};
    Point3D p14{4.0, 5.0, 6.0};
    assert(std::abs(SquaredDistance(p13, p14) - SquaredDistance(p14, p13)) < 1e-9);
    
    return 0;
}
