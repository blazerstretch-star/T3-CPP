/*
 * This file is part of cg3lib: https://github.com/cg3hci/cg3lib
 * This Source Code Form is subject to the terms of the GNU GPL 3.0
 *
 * @author Alessandro Muntoni (muntoni.alessandro@gmail.com)
 */

#ifndef CG3_CGAL_TRIANGULATION2_H
#define CG3_CGAL_TRIANGULATION2_H

#include <cg3/geometry/point2.h>

namespace cg3 {
namespace cgal {
namespace internal {

static std::vector< std::vector<Point2d > > dummy_holes2D;

} //namespace cg3::cgal::internal

std::vector<std::array<Point2d , 3> > triangulate2(
        const std::vector<Point2d >& polygon,
        const std::vector<std::vector<Point2d > >& holes = internal::dummy_holes2D);

} //namespace cg3::cgal
} //namespace cg3

#ifndef CG3_STATIC
#define CG3_CGAL_TRIANGULATION2_CPP "triangulation2.cpp"
#include CG3_CGAL_TRIANGULATION2_CPP
#undef CG3_CGAL_TRIANGULATION2_CPP
#endif //CG3_STATIC

#endif // CG3_CGAL_TRIANGULATION2_H

inline std::vector<Point2d> delaunayTriangulation2D(const std::vector<Point2d>& points) {
    // FUNCTION_ID: cg3lib_func010 - START
    Delaunay2 dt(points.begin(), points.end());
    std::vector<Point2d> result;
    for (auto fit = dt.finite_faces_begin(); fit != dt.finite_faces_end(); ++fit) {
        for (int i = 0; i < 3; ++i) result.push_back(fit->vertex(i)->point());
    }
    return result;
    // FUNCTION_ID: cg3lib_func010 - END
}

inline int computeAlphaShape2D(const std::vector<Point2d>& points, double alpha) {
    // FUNCTION_ID: cg3lib_func014 - START
    AlphaShape2 as(points.begin(), points.end(), AlphaShape2::GENERAL);
    as.set_alpha(alpha);
    int count = 0;
    for (auto it = as.alpha_shape_edges_begin(); it != as.alpha_shape_edges_end(); ++it) count++;
    return count;
    // FUNCTION_ID: cg3lib_func014 - END
}
