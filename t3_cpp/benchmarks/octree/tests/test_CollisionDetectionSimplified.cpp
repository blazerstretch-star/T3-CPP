#include "../src/octree_functions.h"
#include <cassert>
#include <algorithm>

using namespace OrthoTree;

int main() {
    // Test 1: Two overlapping boxes
    std::vector<BoundingBox3D> boxes1 = {
        BoundingBox3D{Point3D{0.0, 0.0, 0.0}, Point3D{2.0, 2.0, 2.0}},
        BoundingBox3D{Point3D{1.0, 1.0, 1.0}, Point3D{3.0, 3.0, 3.0}}
    };
    auto result1 = CollisionDetectionSimplified(boxes1);
    assert(result1.size() == 1);
    assert((result1[0].first == 0 && result1[0].second == 1));
    
    // Test 2: No collisions
    std::vector<BoundingBox3D> boxes2 = {
        BoundingBox3D{Point3D{0.0, 0.0, 0.0}, Point3D{1.0, 1.0, 1.0}},
        BoundingBox3D{Point3D{2.0, 2.0, 2.0}, Point3D{3.0, 3.0, 3.0}},
        BoundingBox3D{Point3D{4.0, 4.0, 4.0}, Point3D{5.0, 5.0, 5.0}}
    };
    auto result2 = CollisionDetectionSimplified(boxes2);
    assert(result2.size() == 0);
    
    // Test 3: Multiple collisions
    std::vector<BoundingBox3D> boxes3 = {
        BoundingBox3D{Point3D{0.0, 0.0, 0.0}, Point3D{3.0, 3.0, 3.0}},
        BoundingBox3D{Point3D{1.0, 1.0, 1.0}, Point3D{4.0, 4.0, 4.0}},
        BoundingBox3D{Point3D{2.0, 2.0, 2.0}, Point3D{5.0, 5.0, 5.0}}
    };
    auto result3 = CollisionDetectionSimplified(boxes3);
    assert(result3.size() == 3);
    
    // Test 4: One box contains another
    std::vector<BoundingBox3D> boxes4 = {
        BoundingBox3D{Point3D{0.0, 0.0, 0.0}, Point3D{10.0, 10.0, 10.0}},
        BoundingBox3D{Point3D{2.0, 2.0, 2.0}, Point3D{5.0, 5.0, 5.0}}
    };
    auto result4 = CollisionDetectionSimplified(boxes4);
    assert(result4.size() == 1);
    
    // Test 5: Adjacent boxes (should not collide)
    std::vector<BoundingBox3D> boxes5 = {
        BoundingBox3D{Point3D{0.0, 0.0, 0.0}, Point3D{1.0, 1.0, 1.0}},
        BoundingBox3D{Point3D{1.0, 0.0, 0.0}, Point3D{2.0, 1.0, 1.0}}
    };
    auto result5 = CollisionDetectionSimplified(boxes5);
    assert(result5.size() == 0);
    
    // Test 6: Empty box list
    std::vector<BoundingBox3D> boxes6;
    auto result6 = CollisionDetectionSimplified(boxes6);
    assert(result6.size() == 0);
    
    // Test 7: Single box (no collisions)
    std::vector<BoundingBox3D> boxes7 = {
        BoundingBox3D{Point3D{0.0, 0.0, 0.0}, Point3D{1.0, 1.0, 1.0}}
    };
    auto result7 = CollisionDetectionSimplified(boxes7);
    assert(result7.size() == 0);
    
    return 0;
}
