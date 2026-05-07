#ifndef BOOLEANS2D_H
#define BOOLEANS2D_H

#include <vector>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/Segment_2.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Arr_segment_traits_2.h>

typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
typedef Kernel::Point_2 Point2d;
typedef CGAL::Polygon_2<Kernel> Polygon2d;
typedef CGAL::Polygon_with_holes_2<Kernel> PolygonWithHoles2d;
typedef CGAL::Segment_2<Kernel> Segment2d;
typedef CGAL::Arr_segment_traits_2<Kernel> Traits_2;
typedef CGAL::Arrangement_2<Traits_2> Arrangement2;

// Function declarations
std::vector<PolygonWithHoles2d> difference2D(const Polygon2d& P1, const Polygon2d& P2);
std::vector<PolygonWithHoles2d> intersection2D(const Polygon2d& P1, const Polygon2d& P2);
std::vector<PolygonWithHoles2d> union2D(const Polygon2d& P1, const Polygon2d& P2);
bool doIntersect2D(const Polygon2d& P1, const Polygon2d& P2);
std::vector<Point2d> segmentIntersections(const std::vector<Segment2d>& segments);
Arrangement2 computeArrangement(const std::vector<Segment2d>& segments);
std::vector<Polygon2d> extractArrangementFaces(const Arrangement2& arr);

#endif // BOOLEANS2D_H
