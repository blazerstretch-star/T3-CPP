/*
 * This file is part of cg3lib: https://github.com/cg3hci/cg3lib
 * This Source Code Form is subject to the terms of the GNU GPL 3.0
 *
 * @author Alessandro Muntoni (muntoni.alessandro@gmail.com)
 */
#include "delaunay_triangulation3.h"

namespace cg3 {
namespace cgal {

CG3_INLINE DelaunayTriangulation3::DelaunayTriangulation3()
{
}

CG3_INLINE void DelaunayTriangulation3::pushPoint(const Point3d& p)
{
    // FUNCTION_ID: cg3lib_func012 - START
    triangulation.insert(CGALPoint(p.x(), p.y(), p.z()));
    // FUNCTION_ID: cg3lib_func012 - END
}

}
}

CG3_INLINE std::vector<Tetrahedron> DelaunayTriangulation3::extractTetrahedra() const
{
    // FUNCTION_ID: cg3lib_func013 - START
    std::vector<Tetrahedron> tetrahedra;
    for (auto cit = triangulation.finite_cells_begin(); cit != triangulation.finite_cells_end(); ++cit) {
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
