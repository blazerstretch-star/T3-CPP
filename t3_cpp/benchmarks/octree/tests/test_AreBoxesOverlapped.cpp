#include "../src/octree_functions.h"
#include <cassert>

using namespace OrthoTree;

int main() {
    // Test 1: Overlapping boxes (containment mode)
    BoundingBox3D box1{Point3D{0.0, 0.0, 0.0}, Point3D{10.0, 10.0, 10.0}};
    BoundingBox3D box2{Point3D{2.0, 2.0, 2.0}, Point3D{5.0, 5.0, 5.0}};
    assert(AreBoxesOverlapped(box1, box2, true, false) == true);
    
    // Test 2: Non-contained boxes (containment mode should fail)
    BoundingBox3D box3{Point3D{0.0, 0.0, 0.0}, Point3D{5.0, 5.0, 5.0}};
    BoundingBox3D box4{Point3D{3.0, 3.0, 3.0}, Point3D{7.0, 7.0, 7.0}};
    assert(AreBoxesOverlapped(box3, box4, true, false) == false);
    
    // Test 3: Overlapping boxes (non-containment mode)
    assert(AreBoxesOverlapped(box3, box4, false, false) == true);
    
    // Test 4: Separated boxes
    BoundingBox3D box5{Point3D{0.0, 0.0, 0.0}, Point3D{1.0, 1.0, 1.0}};
    BoundingBox3D box6{Point3D{2.0, 2.0, 2.0}, Point3D{3.0, 3.0, 3.0}};
    assert(AreBoxesOverlapped(box5, box6, false, false) == false);
    
    // Test 5: Adjacent boxes (point touch not allowed)
    BoundingBox3D box7{Point3D{0.0, 0.0, 0.0}, Point3D{1.0, 1.0, 1.0}};
    BoundingBox3D box8{Point3D{1.0, 0.0, 0.0}, Point3D{2.0, 1.0, 1.0}};
    assert(AreBoxesOverlapped(box7, box8, false, false) == false);
    
    // Test 6: Adjacent boxes (point touch allowed)
    assert(AreBoxesOverlapped(box7, box8, false, true) == true);
    
    // Test 7: Identical boxes
    BoundingBox3D box9{Point3D{1.0, 1.0, 1.0}, Point3D{2.0, 2.0, 2.0}};
    BoundingBox3D box10{Point3D{1.0, 1.0, 1.0}, Point3D{2.0, 2.0, 2.0}};
    assert(AreBoxesOverlapped(box9, box10, false, false) == true);
    assert(AreBoxesOverlapped(box9, box10, true, false) == true);
    
    return 0;
}
