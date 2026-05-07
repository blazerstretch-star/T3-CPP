#ifndef CONVEX_HULL_H
#define CONVEX_HULL_H

#include <vector>
#include <array>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Point_2.h>
#include <CGAL/Point_3.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Alpha_shape_2.h>
#include <CGAL/Alpha_shape_vertex_base_2.h>
#include <CGAL/Alpha_shape_face_base_2.h>
#include <CGAL/Triangulation_data_structure_2.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef Kernel::Point_2 Point2d;
typedef Kernel::Point_3 Point3d;
typedef CGAL::Polygon_2<Kernel> Polygon2d;
typedef CGAL::Delaunay_triangulation_2<Kernel> Triangulation2;
typedef CGAL::Delaunay_triangulation_3<Kernel> Triangulation3;
typedef std::array<Point3d, 4> Tetrahedron;

typedef CGAL::Alpha_shape_vertex_base_2<Kernel> Vb;
typedef CGAL::Alpha_shape_face_base_2<Kernel> Fb;
typedef CGAL::Triangulation_data_structure_2<Vb,Fb> Tds;
typedef CGAL::Delaunay_triangulation_2<Kernel,Tds> Delaunay;
typedef CGAL::Alpha_shape_2<Delaunay> AlphaShape2;

struct BoundingBox2 {
    Point2d min_corner;
    Point2d max_corner;
};

// Function declarations
std::vector<Point2d> convexHull2D(const std::vector<Point2d>& points);
BoundingBox2 minimumBoundingBox2D(const std::vector<Point2d>& points);
Triangulation2 delaunayTriangulation2D(const std::vector<Point2d>& points);
std::vector<std::vector<Point2d>> voronoiDiagram2D(const std::vector<Point2d>& points);
Triangulation3 delaunayTriangulation3D(const std::vector<Point3d>& points);
std::vector<Tetrahedron> extractTetrahedra(const Triangulation3& tri);
int computeAlphaShape2D(const std::vector<Point2d>& points, double alpha);

#endif // CONVEX_HULL_H
