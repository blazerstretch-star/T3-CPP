#ifndef CGAL_KERNEL_FUNCTIONS_H
#define CGAL_KERNEL_FUNCTIONS_H

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <vector>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef K::Point_2 Point_2;
typedef K::Vector_2 Vector_2;
typedef K::FT FT;
typedef K::Line_2 Line_2;

namespace CGALKernel {

// ============================================================================
// SIMPLE FUNCTIONS (10) - API Wrappers
// ============================================================================

// Compute midpoint between two points
Point_2 midpoint(const Point_2& p, const Point_2& q);

// Check if three points are collinear
bool collinear(const Point_2& p, const Point_2& q, const Point_2& r);

// Check if three points form a left turn
bool left_turn(const Point_2& p, const Point_2& q, const Point_2& r);

// Check if three points form a right turn
bool right_turn(const Point_2& p, const Point_2& q, const Point_2& r);

// Compute signed area of triangle
FT triangle_area(const Point_2& p, const Point_2& q, const Point_2& r);

// Get orientation of three points
CGAL::Orientation orientation(const Point_2& p, const Point_2& q, const Point_2& r);

// Compute squared distance between two points
FT squared_distance(const Point_2& p, const Point_2& q);

// Compute determinant of two vectors
FT determinant(const Vector_2& u, const Vector_2& v);

// Compare x-coordinates of two points
CGAL::Comparison_result compare_x(const Point_2& p, const Point_2& q);

// Compare y-coordinates of two points
CGAL::Comparison_result compare_y(const Point_2& p, const Point_2& q);

// ============================================================================
// COMPLEX FUNCTIONS (5) - Algorithmic Implementations
// ============================================================================

// Compute signed area of polygon (25 lines)
FT polygon_area(const std::vector<Point_2>& polygon);

// Check if polygon is convex (80 lines)
bool is_polygon_convex(const std::vector<Point_2>& polygon);

// Point-in-polygon test using ray casting (105 lines)
CGAL::Bounded_side point_in_polygon(const std::vector<Point_2>& polygon, const Point_2& point);

// Find leftmost point in a set (with tie-breaking) (15 lines)
Point_2 leftmost_point(const std::vector<Point_2>& points);

// Compute convex hull using Graham scan (90 lines)
std::vector<Point_2> convex_hull_graham(std::vector<Point_2> points);

}

#endif
