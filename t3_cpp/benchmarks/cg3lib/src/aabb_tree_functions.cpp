
#include "aabb_tree_functions.h"
#include <random>
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


int numberIntersectedPrimitives(const Tree& tree, const Point3d& p1, const Point3d& p2) {
    // FUNCTION_ID: cg3lib_func001 - START
    CGALPoint pa(p1[0], p1[1], p1[2]); CGALPoint pb(p2[0], p2[1], p2[2]); Kernel::Ray_3 ray_query(pa, pb); return (int) tree.number_of_intersected_primitives(ray_query);
    // FUNCTION_ID: cg3lib_func001 - END
}

int numberIntersectedPrimitives(const Tree& tree, const BoundingBox3& b) {
    // FUNCTION_ID: cg3lib_func002 - START
    Kernel::Iso_cuboid_3 bbox_query(CGALPoint(b.min[0], b.min[1], b.min[2]), CGALPoint(b.max[0], b.max[1], b.max[2])); return (int) tree.number_of_intersected_primitives(bbox_query);
    // FUNCTION_ID: cg3lib_func002 - END
}


double squaredDistance(const Tree& tree, const Point3d& p) {
    // FUNCTION_ID: cg3lib_func003 - START
    CGALPoint query(p[0], p[1], p[2]); return tree.squared_distance(query);
    // FUNCTION_ID: cg3lib_func003 - END
}


Point3d nearestPoint(const Tree& tree, const Point3d& p) {
    // FUNCTION_ID: cg3lib_func004 - START
    CGALPoint query(p[0], p[1], p[2]); CGALPoint closest = tree.closest_point(query); return {closest.x(), closest.y(), closest.z()};
    // FUNCTION_ID: cg3lib_func004 - END
}


bool isInside(const Tree& tree, const Point3d& p, int numberOfChecks) {
    // FUNCTION_ID: cg3lib_func005 - START
    int inside = 0, outside = 0; for (int i = 0; i < numberOfChecks; i++) { Point3d boundingPoint = {100.0, 100.0, 100.0}; int numberIntersected = numberIntersectedPrimitives(tree, p, boundingPoint); if (numberIntersected % 2 == 1) inside++; else outside++; } return inside > outside;
    // FUNCTION_ID: cg3lib_func005 - END
}


bool isInsidePseudoRandom(const Tree& tree, const Point3d& p, int numberOfChecks) {
    // FUNCTION_ID: cg3lib_func006 - START
    static std::random_device rd; static std::mt19937 gen(rd()); std::uniform_real_distribution<> dist(-100.0, 100.0); int inside = 0, outside = 0; for (int i = 0; i < numberOfChecks; i++) { Point3d randomPoint = {dist(gen), dist(gen), dist(gen)}; int numberIntersected = numberIntersectedPrimitives(tree, p, randomPoint); if (numberIntersected % 2 == 1) inside++; else outside++; } return inside > outside;
    // FUNCTION_ID: cg3lib_func006 - END
}


int containedTriangles(const Tree& tree, const BoundingBox3& b) {
    // FUNCTION_ID: cg3lib_func007 - START
    Kernel::Iso_cuboid_3 bbox_query(CGALPoint(b.min[0], b.min[1], b.min[2]), CGALPoint(b.max[0], b.max[1], b.max[2])); return (int) tree.number_of_intersected_primitives(bbox_query);
    // FUNCTION_ID: cg3lib_func007 - END
}
