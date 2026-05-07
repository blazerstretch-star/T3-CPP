#ifndef MESH_PROCESSING_H
#define MESH_PROCESSING_H

#include <vector>
#include <array>
#include <string>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Surface_mesh.h>

// Type definitions
typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef CGAL::Polyhedron_3<Kernel> Polyhedron;
typedef CGAL::Surface_mesh<Kernel::Point_3> SurfaceMesh;
typedef Kernel::Point_3 Point3d;
typedef Kernel::Point_2 Point2d;

// Simple mesh structure (simplified from cg3lib)
struct SimpleMesh {
    std::vector<Point3d> vertices;
    std::vector<std::array<int, 3>> faces;
};

// Function declarations
void holeFilling(Polyhedron& poly);

#endif // MESH_PROCESSING_H
