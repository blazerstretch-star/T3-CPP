#include "../src/octree_functions.h"
#include <cassert>

using namespace OrthoTree;

int main() {
    BoundingBox3D box{Point3D{0.0, 0.0, 0.0}, Point3D{10.0, 10.0, 10.0}};
    
    // Test 1: Point inside box
    assert(IsPointInBox(Point3D{5.0, 5.0, 5.0}, box) == true);
    
    // Test 2: Point at corner (min)
    assert(IsPointInBox(Point3D{0.0, 0.0, 0.0}, box) == true);
    
    // Test 3: Point at corner (max)
    assert(IsPointInBox(Point3D{10.0, 10.0, 10.0}, box) == true);
    
    // Test 4: Point outside (x too large)
    assert(IsPointInBox(Point3D{10.1, 5.0, 5.0}, box) == false);
    
    // Test 5: Point outside (y too small)
    assert(IsPointInBox(Point3D{5.0, -0.1, 5.0}, box) == false);
    
    // Test 6: Point outside (z too large)
    assert(IsPointInBox(Point3D{5.0, 5.0, 10.001}, box) == false);
    
    // Test 7: Point on edge (x min)
    assert(IsPointInBox(Point3D{0.0, 5.0, 5.0}, box) == true);
    
    // Test 8: Point on edge (y max)
    assert(IsPointInBox(Point3D{5.0, 10.0, 5.0}, box) == true);
    
    // Test 9: Point far outside
    assert(IsPointInBox(Point3D{100.0, 100.0, 100.0}, box) == false);
    
    // Test 10: Negative coordinates box
    BoundingBox3D box2{Point3D{-5.0, -5.0, -5.0}, Point3D{5.0, 5.0, 5.0}};
    assert(IsPointInBox(Point3D{0.0, 0.0, 0.0}, box2) == true);
    assert(IsPointInBox(Point3D{-5.0, -5.0, -5.0}, box2) == true);
    assert(IsPointInBox(Point3D{-5.1, 0.0, 0.0}, box2) == false);
    
    return 0;
}
