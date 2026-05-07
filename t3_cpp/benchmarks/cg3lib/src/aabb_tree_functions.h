#pragma once
#include <array>
#include <vector>
#include <list>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_triangle_primitive.h>

using Kernel = CGAL::Simple_cartesian<double>;
using Point3d = std::array<double, 3>;
using CGALPoint = Kernel::Point_3;
using CGALTriangle = Kernel::Triangle_3;

struct BoundingBox3 {
    Point3d min;
    Point3d max;
};

using TriangleList = std::vector<CGALTriangle>;
using Iterator = TriangleList::iterator;
using Primitive = CGAL::AABB_triangle_primitive<Kernel, Iterator>;
using Traits = CGAL::AABB_traits<Kernel, Primitive>;
using Tree = CGAL::AABB_tree<Traits>;

int numberIntersectedPrimitives(const Tree& tree, const Point3d& p1, const Point3d& p2);
int numberIntersectedPrimitives(const Tree& tree, const BoundingBox3& b);
double squaredDistance(const Tree& tree, const Point3d& p);
Point3d nearestPoint(const Tree& tree, const Point3d& p);
bool isInside(const Tree& tree, const Point3d& p, int numberOfChecks);
bool isInsidePseudoRandom(const Tree& tree, const Point3d& p, int numberOfChecks);
int containedTriangles(const Tree& tree, const BoundingBox3& b);
