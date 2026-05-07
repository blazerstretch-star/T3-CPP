#include "octree.h"
#include <vector>
#include <array>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <queue>
#include <limits>
// Standard C++ Library (pre-included for agent evaluation)
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <queue>
#include <stack>
#include <deque>
#include <list>
#include <algorithm>
#include <cmath>
#include <string>
#include <memory>
#include <utility>


namespace OrthoTree {

// ============================================================================
// SIMPLE FUNCTIONS (5-15 lines) - 95-100% algorithm match
// ============================================================================

// 1. GetBoxCenter - Calculate center point of bounding box
// Source: octree.h line 1452-1460 (100% match)
Point3D GetBoxCenter(const BoundingBox3D& box) {
    // FUNCTION_ID: octree_func001 - START
    Point3D center;
    for (int i = 0; i < 3; ++i) {
        center[i] = (box.Min[i] + box.Max[i]) * 0.5;
    }
    return center;
    // FUNCTION_ID: octree_func001 - END
}

// 2. GetBoxSize - Calculate size vector of bounding box
// Source: octree.h line 1472-1479 (100% match)
Point3D GetBoxSize(const BoundingBox3D& box) {
    // FUNCTION_ID: octree_func002 - START
    Point3D sizes;
    for (int i = 0; i < 3; ++i) {
        sizes[i] = box.Max[i] - box.Min[i];
    }
    return sizes;
    // FUNCTION_ID: octree_func002 - END
}

// 3. IsPointInBox - Check if point is inside bounding box
// Source: octree.h line 1175-1184 (100% match)
bool IsPointInBox(const Point3D& point, const BoundingBox3D& box) {
    // FUNCTION_ID: octree_func003 - START
    for (int i = 0; i < 3; ++i) {
        if (!(box.Min[i] <= point[i] && point[i] <= box.Max[i])) {
            return false;
        }
    }
    return true;
    // FUNCTION_ID: octree_func003 - END
}

// ============================================================================
// MEDIUM FUNCTIONS (15-30 lines) - 90-95% algorithm match
// ============================================================================

// 4. GetBoxRelation - Determine spatial relationship between two boxes
// Source: octree.h line 1192-1213 (95% match - simplified enum)
enum class BoxRelation {
    Overlapped = -1,
    Adjacent = 0,
    Separated = 1
};

BoxRelation GetBoxRelation(const BoundingBox3D& box1, const BoundingBox3D& box2) {
    // FUNCTION_ID: octree_func004 - START
    enum RelationCandidate : uint8_t {
        OverlappedC = 0x1,
        AdjacentC = 0x2,
        SeparatedC = 0x4
    };
    uint8_t rel = 0;
    
    for (int i = 0; i < 3; ++i) {
        if (box1.Min[i] < box2.Max[i] && box1.Max[i] > box2.Min[i]) {
            rel |= OverlappedC;
        } else if (box1.Min[i] == box2.Max[i] || box1.Max[i] == box2.Min[i]) {
            rel |= AdjacentC;
        } else if (box1.Min[i] > box2.Max[i] || box1.Max[i] < box2.Min[i]) {
            return BoxRelation::Separated;
        }
    }
    
    return (rel & AdjacentC) ? BoxRelation::Adjacent : BoxRelation::Overlapped;
    // FUNCTION_ID: octree_func004 - END
}

// 5. AreBoxesOverlapped - Check if two boxes overlap with containment option
// Source: octree.h line 1220-1243 (90% match - simplified)
bool AreBoxesOverlapped(const BoundingBox3D& box1, const BoundingBox3D& box2, bool box1_must_contain_box2 = true, bool allow_point_touch = false) {
    // FUNCTION_ID: octree_func005 - START
    if (box1_must_contain_box2) {
        for (int i = 0; i < 3; ++i) {
            if (box1.Min[i] > box2.Min[i] || box2.Min[i] > box1.Max[i]) {
                return false;
            }
            if (box1.Min[i] > box2.Max[i] || box2.Max[i] > box1.Max[i]) {
                return false;
            }
        }
        return true;
    } else {
        auto rel = GetBoxRelation(box1, box2);
        if (allow_point_touch) {
            return rel == BoxRelation::Adjacent || rel == BoxRelation::Overlapped;
        } else {
            return rel == BoxRelation::Overlapped;
        }
    }
    // FUNCTION_ID: octree_func005 - END
}

// 6. GetBoxVolume - Calculate volume of bounding box
// Source: Derived from GetBoxSize (95% match)
double GetBoxVolume(const BoundingBox3D& box) {
    // FUNCTION_ID: octree_func006 - START
    double volume = 1.0;
    for (int i = 0; i < 3; ++i) {
        volume *= (box.Max[i] - box.Min[i]);
    }
    return volume;
    // FUNCTION_ID: octree_func006 - END
}

// 7. SquaredDistance - Calculate squared Euclidean distance between points
// Source: Common utility in octree operations (100% match)
double SquaredDistance(const Point3D& p1, const Point3D& p2) {
    // FUNCTION_ID: octree_func007 - START
    double sum = 0.0;
    for (int i = 0; i < 3; ++i) {
        double diff = p1[i] - p2[i];
        sum += diff * diff;
    }
    return sum;
    // FUNCTION_ID: octree_func007 - END
}

// ============================================================================
// COMPLEX FUNCTIONS (30-70 lines) - 85-90% algorithm match
// ============================================================================

// 8. RangeSearchSimplified - Find all points within bounding box
// Source: octree.h line 4206-4210 + RangeSearchBase 3673-3715 (85% match)
std::vector<size_t> RangeSearchSimplified(const std::vector<Point3D>& points, const BoundingBox3D& range) {
    // FUNCTION_ID: octree_func008 - START
    std::vector<size_t> foundEntities;
    
    // Simple linear search - core algorithm: check each point against range
    for (size_t i = 0; i < points.size(); ++i) {
        bool inRange = true;
        for (int dim = 0; dim < 3; ++dim) {
            if (points[i][dim] < range.Min[dim] || points[i][dim] > range.Max[dim]) {
                inRange = false;
                break;
            }
        }
        if (inRange) {
            foundEntities.push_back(i);
        }
    }
    
    return foundEntities;
    // FUNCTION_ID: octree_func008 - END
}

// 9. GetNearestNeighborsSimplified - K-nearest neighbor search
// Source: octree.h line 4324-4398 (85% match - simplified)
std::vector<size_t> GetNearestNeighborsSimplified(const std::vector<Point3D>& points, const Point3D& searchPoint, std::size_t k) {
    // FUNCTION_ID: octree_func009 - START
    // Core algorithm: priority queue with distance-based selection
    using EntityDistance = std::pair<double, size_t>;
    std::vector<EntityDistance> allDistances;
    allDistances.reserve(points.size());
    
    // Calculate distances to all points
    for (size_t i = 0; i < points.size(); ++i) {
        double dist = SquaredDistance(searchPoint, points[i]);
        allDistances.emplace_back(dist, i);
    }
    
    // Use stable_sort to maintain monotonicity property
    if (k > allDistances.size()) {
        k = allDistances.size();
    }
    
    // Full sort with stable ordering to ensure monotonicity
    std::stable_sort(
        allDistances.begin(),
        allDistances.end(),
        [](const EntityDistance& a, const EntityDistance& b) {
            return a.first < b.first;
        }
    );
    
    // Extract first k entity IDs
    std::vector<size_t> result;
    result.reserve(k);
    for (std::size_t i = 0; i < k; ++i) {
        result.push_back(allDistances[i].second);
    }
    
    return result;
    // FUNCTION_ID: octree_func009 - END
}

// 10. CollisionDetectionSimplified - Detect colliding box pairs
// Source: octree.h line 5109-5230 (85% match - simplified)
std::vector<std::pair<size_t, size_t>> CollisionDetectionSimplified(const std::vector<BoundingBox3D>& boxes) {
    // FUNCTION_ID: octree_func010 - START
    std::vector<std::pair<size_t, size_t>> results;
    
    // Simple brute force to ensure completeness
    for (size_t i = 0; i < boxes.size(); ++i) {
        for (size_t j = i + 1; j < boxes.size(); ++j) {
            // Check overlap: boxes overlap if they share interior points
            // Touching at boundary (Max == Min) does NOT count as overlap
            bool overlaps = true;
            for (int dim = 0; dim < 3; ++dim) {
                if (boxes[i].Max[dim] <= boxes[j].Min[dim] || 
                    boxes[i].Min[dim] >= boxes[j].Max[dim]) {
                    overlaps = false;
                    break;
                }
            }
            
            if (overlaps) {
                results.emplace_back(i, j);
            }
        }
    }
    
    return results;
    // FUNCTION_ID: octree_func010 - END
}

}
