#include "../src/octree_functions.h"
#include <cassert>
#include <cmath>

using namespace OrthoTree;

int main() {
    // Test 1: Standard box
    BoundingBox3D box1{Point3D{0.0, 0.0, 0.0}, Point3D{2.0, 4.0, 6.0}};
    Point3D center1 = GetBoxCenter(box1);
    assert(std::abs(center1[0] - 1.0) < 1e-9);
    assert(std::abs(center1[1] - 2.0) < 1e-9);
    assert(std::abs(center1[2] - 3.0) < 1e-9);
    
    // Test 2: Negative coordinates
    BoundingBox3D box2{Point3D{-4.0, -6.0, -8.0}, Point3D{0.0, 0.0, 0.0}};
    Point3D center2 = GetBoxCenter(box2);
    assert(std::abs(center2[0] - (-2.0)) < 1e-9);
    assert(std::abs(center2[1] - (-3.0)) < 1e-9);
    assert(std::abs(center2[2] - (-4.0)) < 1e-9);
    
    // Test 3: Mixed positive/negative
    BoundingBox3D box3{Point3D{-1.0, -2.0, -3.0}, Point3D{1.0, 2.0, 3.0}};
    Point3D center3 = GetBoxCenter(box3);
    assert(std::abs(center3[0]) < 1e-9);
    assert(std::abs(center3[1]) < 1e-9);
    assert(std::abs(center3[2]) < 1e-9);
    
    // Test 4: Point box (zero volume)
    BoundingBox3D box4{Point3D{5.0, 5.0, 5.0}, Point3D{5.0, 5.0, 5.0}};
    Point3D center4 = GetBoxCenter(box4);
    assert(std::abs(center4[0] - 5.0) < 1e-9);
    assert(std::abs(center4[1] - 5.0) < 1e-9);
    assert(std::abs(center4[2] - 5.0) < 1e-9);
    
    // Test 5: Large coordinates
    BoundingBox3D box5{Point3D{1000.0, 2000.0, 3000.0}, Point3D{1100.0, 2200.0, 3300.0}};
    Point3D center5 = GetBoxCenter(box5);
    assert(std::abs(center5[0] - 1050.0) < 1e-9);
    assert(std::abs(center5[1] - 2100.0) < 1e-9);
    assert(std::abs(center5[2] - 3150.0) < 1e-9);
    
    return 0;
}
