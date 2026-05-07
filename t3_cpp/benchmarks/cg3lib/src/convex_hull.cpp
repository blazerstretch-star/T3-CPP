#include "convex_hull.h"
#include <CGAL/convex_hull_2.h>
#include <CGAL/min_quadrilateral_2.h>
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


std::vector<Point2d> convexHull2D(const std::vector<Point2d>& points) {
    // FUNCTION_ID: cg3lib_func008 - START
    std::vector<Point2d> hull;
    CGAL::convex_hull_2(points.begin(), points.end(), std::back_inserter(hull));
    return hull;
    // FUNCTION_ID: cg3lib_func008 - END
}

BoundingBox2 minimumBoundingBox2D(const std::vector<Point2d>& points) {
    // FUNCTION_ID: cg3lib_func009 - START
    Polygon2d hull;
    CGAL::convex_hull_2(points.begin(), points.end(), std::back_inserter(hull));
    Polygon2d min_rect;
    CGAL::min_rectangle_2(hull.vertices_begin(), hull.vertices_end(), std::back_inserter(min_rect));
    BoundingBox2 bbox;
    bbox.min_corner = min_rect[0];
    bbox.max_corner = min_rect[2];
    return bbox;
    // FUNCTION_ID: cg3lib_func009 - END
}

Triangulation2 delaunayTriangulation2D(const std::vector<Point2d>& points) {
    // FUNCTION_ID: cg3lib_func010 - START
    Triangulation2 tri;
    tri.insert(points.begin(), points.end());
    return tri;
    // FUNCTION_ID: cg3lib_func010 - END
}

std::vector<std::vector<Point2d>> voronoiDiagram2D(const std::vector<Point2d>& points) {
    // FUNCTION_ID: cg3lib_func011 - START
    Triangulation2 tri;
    tri.insert(points.begin(), points.end());
    std::vector<std::vector<Point2d>> voronoi_cells;
    for (auto vit = tri.finite_vertices_begin(); vit != tri.finite_vertices_end(); ++vit) {
        std::vector<Point2d> cell;
        auto circ = tri.incident_faces(vit);
        auto done = circ;
        do {
            if (!tri.is_infinite(circ)) {
                cell.push_back(tri.dual(circ));
            }
            ++circ;
        } while (circ != done);
        voronoi_cells.push_back(cell);
    }
    return voronoi_cells;
    // FUNCTION_ID: cg3lib_func011 - END
}

Triangulation3 delaunayTriangulation3D(const std::vector<Point3d>& points) {
    // FUNCTION_ID: cg3lib_func012 - START
    Triangulation3 tri;
    tri.insert(points.begin(), points.end());
    return tri;
    // FUNCTION_ID: cg3lib_func012 - END
}

std::vector<Tetrahedron> extractTetrahedra(const Triangulation3& tri) {
    // FUNCTION_ID: cg3lib_func013 - START
    std::vector<Tetrahedron> tetrahedra;
    for (auto cit = tri.finite_cells_begin(); cit != tri.finite_cells_end(); ++cit) {
        Tetrahedron tet;
        tet[0] = cit->vertex(0)->point();
        tet[1] = cit->vertex(1)->point();
        tet[2] = cit->vertex(2)->point();
        tet[3] = cit->vertex(3)->point();
        tetrahedra.push_back(tet);
    }
    return tetrahedra;
    // FUNCTION_ID: cg3lib_func013 - END
}

int computeAlphaShape2D(const std::vector<Point2d>& points, double alpha) {
    // FUNCTION_ID: cg3lib_func014 - START
    AlphaShape2 as(points.begin(), points.end(), AlphaShape2::GENERAL);
    as.set_alpha(alpha);
    as.set_mode(AlphaShape2::GENERAL);
    return as.number_of_solid_components();
    // FUNCTION_ID: cg3lib_func014 - END
}
