#ifndef GEOMETRY_UTILS_H
#define GEOMETRY_UTILS_H

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Point_3.h>
#include <CGAL/Vector_3.h>
#include <CGAL/Bbox_3.h>
#include <vector>

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef Kernel::Point_3 Point3d;
typedef Kernel::Vector_3 Vector3d;
typedef CGAL::Bbox_3 BBox3d;

// Function declarations
double pointDistance(const Point3d& p1, const Point3d& p2);
Point3d pointMidpoint(const Point3d& p1, const Point3d& p2);
Vector3d vectorNormalize(const Vector3d& v);
double vectorLength(const Vector3d& v);
BBox3d boundingBox(const std::vector<Point3d>& points);

#endif // GEOMETRY_UTILS_H
