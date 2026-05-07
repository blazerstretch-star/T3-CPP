#include "../src/octree_functions.h"
#include <cassert>

using namespace OrthoTree;

int main() {
    // Test 1: Overlapping boxes
    BoundingBox3D box1{Point3D{0.0, 0.0, 0.0}, Point3D{5.0, 5.0, 5.0}};
    BoundingBox3D box2{Point3D{2.0, 2.0, 2.0}, Point3D{7.0, 7.0, 7.0}};
    assert(GetBoxRelation(box1, box2) == BoxRelation::Overlapped);
    
    // Test 2: Separated boxes
    BoundingBox3D box3{Point3D{0.0, 0.0, 0.0}, Point3D{1.0, 1.0, 1.0}};
    BoundingBox3D box4{Point3D{2.0, 2.0, 2.0}, Point3D{3.0, 3.0, 3.0}};
    assert(GetBoxRelation(box3, box4) == BoxRelation::Separated);
    
    // Test 3: Adjacent boxes (touching)
    BoundingBox3D box5{Point3D{0.0, 0.0, 0.0}, Point3D{1.0, 1.0, 1.0}};
    BoundingBox3D box6{Point3D{1.0, 0.0, 0.0}, Point3D{2.0, 1.0, 1.0}};
    assert(GetBoxRelation(box5, box6) == BoxRelation::Adjacent);
    
    // Test 4: One box contains another (should be overlapped)
    BoundingBox3D box7{Point3D{0.0, 0.0, 0.0}, Point3D{10.0, 10.0, 10.0}};
    BoundingBox3D box8{Point3D{2.0, 2.0, 2.0}, Point3D{5.0, 5.0, 5.0}};
    assert(GetBoxRelation(box7, box8) == BoxRelation::Overlapped);
    
    // Test 5: Identical boxes
    BoundingBox3D box9{Point3D{1.0, 1.0, 1.0}, Point3D{2.0, 2.0, 2.0}};
    BoundingBox3D box10{Point3D{1.0, 1.0, 1.0}, Point3D{2.0, 2.0, 2.0}};
    assert(GetBoxRelation(box9, box10) == BoxRelation::Overlapped);
    
    // Test 6: Adjacent on one face only
    BoundingBox3D box11{Point3D{0.0, 0.0, 0.0}, Point3D{1.0, 1.0, 1.0}};
    BoundingBox3D box12{Point3D{1.0, 1.0, 1.0}, Point3D{2.0, 2.0, 2.0}};
    assert(GetBoxRelation(box11, box12) == BoxRelation::Adjacent);
    
    // Test 7: Separated by large distance
    BoundingBox3D box13{Point3D{0.0, 0.0, 0.0}, Point3D{1.0, 1.0, 1.0}};
    BoundingBox3D box14{Point3D{100.0, 100.0, 100.0}, Point3D{101.0, 101.0, 101.0}};
    assert(GetBoxRelation(box13, box14) == BoxRelation::Separated);
    
    return 0;
}
