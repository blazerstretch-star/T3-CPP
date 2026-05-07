#include "../src/octree_functions.h"
#include <cassert>
#include <cmath>

using namespace OrthoTree;

int main() {
    // Test 1: Unit cube
    BoundingBox3D box1{Point3D{0.0, 0.0, 0.0}, Point3D{1.0, 1.0, 1.0}};
    assert(std::abs(GetBoxVolume(box1) - 1.0) < 1e-9);
    
    // Test 2: 2x3x4 box
    BoundingBox3D box2{Point3D{0.0, 0.0, 0.0}, Point3D{2.0, 3.0, 4.0}};
    assert(std::abs(GetBoxVolume(box2) - 24.0) < 1e-9);
    
    // Test 3: Point box (zero volume)
    BoundingBox3D box3{Point3D{5.0, 5.0, 5.0}, Point3D{5.0, 5.0, 5.0}};
    assert(std::abs(GetBoxVolume(box3)) < 1e-9);
    
    // Test 4: Negative coordinates (volume should be positive)
    BoundingBox3D box4{Point3D{-2.0, -3.0, -4.0}, Point3D{0.0, 0.0, 0.0}};
    assert(std::abs(GetBoxVolume(box4) - 24.0) < 1e-9);
    
    // Test 5: Large box
    BoundingBox3D box5{Point3D{0.0, 0.0, 0.0}, Point3D{10.0, 10.0, 10.0}};
    assert(std::abs(GetBoxVolume(box5) - 1000.0) < 1e-9);
    
    // Test 6: Non-uniform dimensions
    BoundingBox3D box6{Point3D{1.0, 2.0, 3.0}, Point3D{2.0, 2.5, 3.1}};
    assert(std::abs(GetBoxVolume(box6) - 0.05) < 1e-9);
    
    // Test 7: Flat box (one dimension zero)
    BoundingBox3D box7{Point3D{0.0, 0.0, 0.0}, Point3D{5.0, 5.0, 0.0}};
    assert(std::abs(GetBoxVolume(box7)) < 1e-9);
    
    return 0;
}
