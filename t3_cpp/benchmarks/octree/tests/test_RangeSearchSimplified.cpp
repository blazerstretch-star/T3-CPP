#include "../src/octree_functions.h"
#include <cassert>
#include <algorithm>

using namespace OrthoTree;

int main() {
    std::vector<Point3D> points = {
        Point3D{0.0, 0.0, 0.0},
        Point3D{1.0, 1.0, 1.0},
        Point3D{2.0, 2.0, 2.0},
        Point3D{3.0, 3.0, 3.0},
        Point3D{5.0, 5.0, 5.0},
        Point3D{10.0, 10.0, 10.0}
    };
    
    // Test 1: Range contains some points
    BoundingBox3D range1{Point3D{0.0, 0.0, 0.0}, Point3D{2.5, 2.5, 2.5}};
    auto result1 = RangeSearchSimplified(points, range1);
    std::sort(result1.begin(), result1.end());
    assert(result1.size() == 3);
    assert(result1[0] == 0 && result1[1] == 1 && result1[2] == 2);
    
    // Test 2: Range contains all points
    BoundingBox3D range2{Point3D{-1.0, -1.0, -1.0}, Point3D{11.0, 11.0, 11.0}};
    auto result2 = RangeSearchSimplified(points, range2);
    assert(result2.size() == 6);
    
    // Test 3: Range contains no points
    BoundingBox3D range3{Point3D{20.0, 20.0, 20.0}, Point3D{30.0, 30.0, 30.0}};
    auto result3 = RangeSearchSimplified(points, range3);
    assert(result3.size() == 0);
    
    // Test 4: Range contains single point
    BoundingBox3D range4{Point3D{0.9, 0.9, 0.9}, Point3D{1.1, 1.1, 1.1}};
    auto result4 = RangeSearchSimplified(points, range4);
    assert(result4.size() == 1);
    assert(result4[0] == 1);
    
    // Test 5: Range at boundary
    BoundingBox3D range5{Point3D{0.0, 0.0, 0.0}, Point3D{1.0, 1.0, 1.0}};
    auto result5 = RangeSearchSimplified(points, range5);
    assert(result5.size() == 2);
    
    // Test 6: Empty point set
    std::vector<Point3D> empty_points;
    auto result6 = RangeSearchSimplified(empty_points, range1);
    assert(result6.size() == 0);
    
    return 0;
}
