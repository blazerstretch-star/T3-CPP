#include "../src/octree_functions.h"
#include <cassert>
#include <cmath>

using namespace OrthoTree;

int main() {
    // Test 1: Standard box
    BoundingBox3D box1{Point3D{0.0, 0.0, 0.0}, Point3D{2.0, 4.0, 6.0}};
    Point3D size1 = GetBoxSize(box1);
    assert(std::abs(size1[0] - 2.0) < 1e-9);
    assert(std::abs(size1[1] - 4.0) < 1e-9);
    assert(std::abs(size1[2] - 6.0) < 1e-9);
    
    // Test 2: Unit cube
    BoundingBox3D box2{Point3D{0.0, 0.0, 0.0}, Point3D{1.0, 1.0, 1.0}};
    Point3D size2 = GetBoxSize(box2);
    assert(std::abs(size2[0] - 1.0) < 1e-9);
    assert(std::abs(size2[1] - 1.0) < 1e-9);
    assert(std::abs(size2[2] - 1.0) < 1e-9);
    
    // Test 3: Negative coordinates (size should be positive)
    BoundingBox3D box3{Point3D{-5.0, -10.0, -15.0}, Point3D{-2.0, -4.0, -6.0}};
    Point3D size3 = GetBoxSize(box3);
    assert(std::abs(size3[0] - 3.0) < 1e-9);
    assert(std::abs(size3[1] - 6.0) < 1e-9);
    assert(std::abs(size3[2] - 9.0) < 1e-9);
    
    // Test 4: Point box (zero size)
    BoundingBox3D box4{Point3D{3.0, 3.0, 3.0}, Point3D{3.0, 3.0, 3.0}};
    Point3D size4 = GetBoxSize(box4);
    assert(std::abs(size4[0]) < 1e-9);
    assert(std::abs(size4[1]) < 1e-9);
    assert(std::abs(size4[2]) < 1e-9);
    
    // Test 5: Non-uniform dimensions
    BoundingBox3D box5{Point3D{1.0, 2.0, 3.0}, Point3D{1.5, 2.1, 3.001}};
    Point3D size5 = GetBoxSize(box5);
    assert(std::abs(size5[0] - 0.5) < 1e-9);
    assert(std::abs(size5[1] - 0.1) < 1e-9);
    assert(std::abs(size5[2] - 0.001) < 1e-9);
    
    return 0;
}
