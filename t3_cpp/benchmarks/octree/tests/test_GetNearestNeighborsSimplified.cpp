#include "../src/octree_functions.h"
#include <cassert>

using namespace OrthoTree;

int main() {
    std::vector<Point3D> points = {
        Point3D{0.0, 0.0, 0.0},
        Point3D{1.0, 1.0, 1.0},
        Point3D{2.0, 2.0, 2.0},
        Point3D{3.0, 3.0, 3.0},
        Point3D{10.0, 10.0, 10.0}
    };
    
    // Test 1: Find 1 nearest neighbor
    Point3D search1{1.1, 1.1, 1.1};
    auto result1 = GetNearestNeighborsSimplified(points, search1, 1);
    assert(result1.size() == 1);
    assert(result1[0] == 1);
    
    // Test 2: Find 3 nearest neighbors
    Point3D search2{0.0, 0.0, 0.0};
    auto result2 = GetNearestNeighborsSimplified(points, search2, 3);
    assert(result2.size() == 3);
    assert(result2[0] == 0);
    
    // Test 3: k larger than point count
    auto result3 = GetNearestNeighborsSimplified(points, search2, 10);
    assert(result3.size() == 5);
    
    // Test 4: k = 0 (edge case)
    auto result4 = GetNearestNeighborsSimplified(points, search2, 0);
    assert(result4.size() == 0);
    
    // Test 5: Search point far from all
    Point3D search5{100.0, 100.0, 100.0};
    auto result5 = GetNearestNeighborsSimplified(points, search5, 1);
    assert(result5.size() == 1);
    assert(result5[0] == 4);
    
    // Test 6: Search point at exact location
    Point3D search6{2.0, 2.0, 2.0};
    auto result6 = GetNearestNeighborsSimplified(points, search6, 1);
    assert(result6.size() == 1);
    assert(result6[0] == 2);
    
    // Test 7: Find all neighbors
    auto result7 = GetNearestNeighborsSimplified(points, search2, 5);
    assert(result7.size() == 5);
    
    return 0;
}
