#ifndef OCTREE_FUNCTIONS_H
#define OCTREE_FUNCTIONS_H

#include "octree.h"

namespace OrthoTree {

enum class BoxRelation {
    Overlapped = -1,
    Adjacent = 0,
    Separated = 1
};

Point3D GetBoxCenter(const BoundingBox3D& box);
Vector3D GetBoxSize(const BoundingBox3D& box);
bool IsPointInBox(const Point3D& point, const BoundingBox3D& box);
BoxRelation GetBoxRelation(const BoundingBox3D& box1, const BoundingBox3D& box2);
bool AreBoxesOverlapped(const BoundingBox3D& box1, const BoundingBox3D& box2, bool box1_must_contain_box2 = true, bool allow_point_touch = false);
double GetBoxVolume(const BoundingBox3D& box);
double SquaredDistance(const Point3D& p1, const Point3D& p2);
std::vector<size_t> RangeSearchSimplified(const std::vector<Point3D>& points, const BoundingBox3D& range);
std::vector<size_t> GetNearestNeighborsSimplified(const std::vector<Point3D>& points, const Point3D& target, size_t k);
std::vector<std::pair<size_t, size_t>> CollisionDetectionSimplified(const std::vector<BoundingBox3D>& boxes);

}

#endif
