#ifndef CGAL_FUNCTIONS_H
#define CGAL_FUNCTIONS_H

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/Triangulation_face_base_with_info_2.h>
#include <CGAL/linear_least_squares_fitting_3.h>
#include <list>
#include <vector>
#include "Enhanced_constrained_triangulation_2.h"

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef CGAL::Exact_predicates_tag Tag;

struct VertexInfo {
  bool has_point;
  Kernel::Point_3 point;
  VertexInfo() : has_point(false), point(CGAL::ORIGIN) {}
};

struct FaceInfo {
  bool processed;
  bool interior;
  FaceInfo() : processed(false), interior(false) {}
};

typedef CGAL::Triangulation_vertex_base_with_info_2<VertexInfo, Kernel> VertexBase;
typedef CGAL::Constrained_triangulation_face_base_2<Kernel> FaceBase;
typedef CGAL::Triangulation_face_base_with_info_2<FaceInfo, Kernel, FaceBase> FaceBaseWithInfo;
typedef CGAL::Triangulation_data_structure_2<VertexBase, FaceBaseWithInfo> TriangulationDataStructure;
typedef CGAL::Constrained_Delaunay_triangulation_2<Kernel, TriangulationDataStructure, Tag> ConstrainedDelaunayTriangulation;
typedef Enhanced_constrained_triangulation_2<ConstrainedDelaunayTriangulation> Triangulation;

namespace CGALFunctions {
    // Extract 3D points from coordinate arrays
    void extract_points_from_coordinates(const std::vector<std::vector<double>>& coords, 
                                       std::list<Kernel::Point_3>& points);
    
    // Compute best fitting plane using CGAL least squares
    Kernel::Plane_3 compute_best_fitting_plane(const std::list<Kernel::Point_3>& points);
    
    // Insert polygon constraints into triangulation
    void insert_polygon_constraints(const std::vector<std::vector<double>>& polygon_coords,
                                  Triangulation& triangulation, 
                                  const Kernel::Plane_3& plane,
                                  Triangulation::Face_handle& walk_start);
    
    // Label triangulation faces as interior/exterior
    void label_triangulation_faces(Triangulation& triangulation);
    
    // Check if triangulation edge is constrained
    bool is_triangulation_edge_constrained(const Triangulation& triangulation, 
                                         Triangulation::Face_handle face, int edge_index);
    
    // Get boundary vertices from face using recursive traversal
    void get_face_boundary_vertices(Triangulation::Face_handle face, int edge,
                                  std::list<Triangulation::Vertex_handle>& vertices);
}

#endif