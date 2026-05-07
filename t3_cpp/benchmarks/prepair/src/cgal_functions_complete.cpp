#include "cgal_functions.h"
#include <iostream>
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


namespace CGALFunctions {
    
    void extract_points_from_coordinates(const std::vector<std::vector<double>>& coords, std::list<Kernel::Point_3>& points) {
        // FUNCTION_ID: prepair_func001 - START
        points.clear();
        for (const auto& coord : coords) {
            if (coord.size() >= 3) {
                points.push_back(Kernel::Point_3(coord[0], coord[1], coord[2]));
            } else if (coord.size() >= 2) {
                points.push_back(Kernel::Point_3(coord[0], coord[1], 0.0));
            }
        }
        // FUNCTION_ID: prepair_func001 - END
    }
    
    Kernel::Plane_3 compute_best_fitting_plane(const std::list<Kernel::Point_3>& points) {
        // FUNCTION_ID: prepair_func002 - START
        if (points.size() < 3) {
            return Kernel::Plane_3(Kernel::Point_3(0,0,0), Kernel::Vector_3(0,0,1));
        }
        
        Kernel::Plane_3 best_plane;
        CGAL::linear_least_squares_fitting_3(points.begin(), points.end(), 
                                            best_plane, CGAL::Dimension_tag<0>());
        return best_plane;
        // FUNCTION_ID: prepair_func002 - END
    }
    
    void insert_polygon_constraints(const std::vector<std::vector<double>>& polygon_coords, Triangulation& triangulation, const Kernel::Plane_3& plane, Triangulation::Face_handle& walk_start) {
        // FUNCTION_ID: prepair_func003 - START
        if (polygon_coords.empty()) return;
        
        // Handle degenerate cases
        if (polygon_coords.size() == 1) {
            // Single point - just insert it
            Kernel::Point_3 pt_3d(polygon_coords[0][0], polygon_coords[0][1], 
                                 polygon_coords[0].size() > 2 ? polygon_coords[0][2] : 0.0);
            Triangulation::Vertex_handle v = triangulation.insert(plane.to_2d(pt_3d), walk_start);
            v->info().has_point = true;
            v->info().point = pt_3d;
            walk_start = triangulation.incident_faces(v);
            return;
        }
        
        if (polygon_coords.size() == 2) {
            // Two points - insert both and create constraint
            Kernel::Point_3 pt1_3d(polygon_coords[0][0], polygon_coords[0][1], 
                                  polygon_coords[0].size() > 2 ? polygon_coords[0][2] : 0.0);
            Kernel::Point_3 pt2_3d(polygon_coords[1][0], polygon_coords[1][1], 
                                  polygon_coords[1].size() > 2 ? polygon_coords[1][2] : 0.0);
            
            Triangulation::Vertex_handle v1 = triangulation.insert(plane.to_2d(pt1_3d), walk_start);
            v1->info().has_point = true;
            v1->info().point = pt1_3d;
            walk_start = triangulation.incident_faces(v1);
            
            Triangulation::Vertex_handle v2 = triangulation.insert(plane.to_2d(pt2_3d), walk_start);
            v2->info().has_point = true;
            v2->info().point = pt2_3d;
            
            if (v1 != v2) {
                triangulation.odd_even_insert_constraint(v1, v2);
            }
            walk_start = triangulation.incident_faces(v2);
            return;
        }
        
        // Normal case: 3+ points forming a polygon
        Triangulation::Vertex_handle va, vb, first_vertex;
        
        // Insert first point
        Kernel::Point_3 first_3d(polygon_coords[0][0], polygon_coords[0][1], 
                                polygon_coords[0].size() > 2 ? polygon_coords[0][2] : 0.0);
        vb = triangulation.insert(plane.to_2d(first_3d), walk_start);
        vb->info().has_point = true;
        vb->info().point = first_3d;
        first_vertex = vb;
        walk_start = triangulation.incident_faces(vb);
        
        // Insert remaining points and constraints
        for (size_t i = 1; i < polygon_coords.size(); ++i) {
            va = vb;
            Kernel::Point_3 new_3d(polygon_coords[i][0], polygon_coords[i][1],
                                 polygon_coords[i].size() > 2 ? polygon_coords[i][2] : 0.0);
            vb = triangulation.insert(plane.to_2d(new_3d), walk_start);
            vb->info().has_point = true;
            vb->info().point = new_3d;
            
            if (va != vb) {
                triangulation.odd_even_insert_constraint(va, vb);
            }
            walk_start = triangulation.incident_faces(vb);
        }
        
        // Close polygon
        if (vb != first_vertex) {
            triangulation.odd_even_insert_constraint(vb, first_vertex);
        }
        // FUNCTION_ID: prepair_func003 - END
    }
    
    void label_triangulation_faces(Triangulation& triangulation) {
        // FUNCTION_ID: prepair_func004 - START
        // Handle empty triangulation
        if (triangulation.number_of_vertices() == 0) {
            // Initialize infinite face for empty triangulation
            triangulation.infinite_face()->info().processed = true;
            triangulation.infinite_face()->info().interior = false;
            return;
        }
        
        // BUG FIX: Initialize all faces first to ensure clean state
        for (auto fit = triangulation.all_faces_begin(); fit != triangulation.all_faces_end(); ++fit) {
            fit->info().processed = false;
            fit->info().interior = false;
        }
        
        std::list<Triangulation::Face_handle> to_check;
        
        // Start from infinite face (always exterior)
        triangulation.infinite_face()->info().processed = true;
        triangulation.infinite_face()->info().interior = false;
        to_check.push_back(triangulation.infinite_face());
        
        // BFS traversal with odd-even rule
        while (!to_check.empty()) {
            Triangulation::Face_handle current_face = to_check.front();
            to_check.pop_front();
            
            for (int neighbour = 0; neighbour < 3; ++neighbour) {
                Triangulation::Face_handle neighbor_face = current_face->neighbor(neighbour);
                
                if (!neighbor_face->info().processed) {
                    neighbor_face->info().processed = true;
                    
                    // Odd-even rule: flip interior/exterior across constraints
                    if (triangulation.is_constrained(Triangulation::Edge(current_face, neighbour))) {
                        neighbor_face->info().interior = !current_face->info().interior;
                    } else {
                        neighbor_face->info().interior = current_face->info().interior;
                    }
                    
                    to_check.push_back(neighbor_face);
                }
            }
        }
        // FUNCTION_ID: prepair_func004 - END
    }
    
    bool is_triangulation_edge_constrained(const Triangulation& triangulation, Triangulation::Face_handle face, int edge_index) {
        // FUNCTION_ID: prepair_func005 - START
        if (edge_index < 0 || edge_index >= 3) return false;
        return triangulation.is_constrained(Triangulation::Edge(face, edge_index));
        // FUNCTION_ID: prepair_func005 - END
    }
    
    void get_face_boundary_vertices(Triangulation::Face_handle face, int edge, std::list<Triangulation::Vertex_handle>& vertices) {
        // FUNCTION_ID: prepair_func006 - START
        // BUG FIX: Validate edge index before use
        if (edge < 0 || edge >= 3) {
            return;  // Gracefully handle invalid edge index
        }
        
        // Check if face is valid for traversal
        if (!face->info().interior || face->info().processed) {
            return;
        }
        
        face->info().processed = true;
        
        // Check clockwise neighbor
        int cw_edge = face->cw(edge);
        Triangulation::Face_handle cw_neighbor = face->neighbor(cw_edge);
        if (cw_neighbor->info().interior && !cw_neighbor->info().processed) {
            int cw_neighbor_edge = cw_neighbor->index(face);
            // Validate neighbor edge index before recursion
            if (cw_neighbor_edge >= 0 && cw_neighbor_edge < 3) {
                std::list<Triangulation::Vertex_handle> cw_vertices;
                get_face_boundary_vertices(cw_neighbor, cw_neighbor_edge, cw_vertices);
                vertices.splice(vertices.end(), cw_vertices);
            }
        }
        
        // Add central vertex
        vertices.push_back(face->vertex(edge));
        
        // Check counterclockwise neighbor
        int ccw_edge = face->ccw(edge);
        Triangulation::Face_handle ccw_neighbor = face->neighbor(ccw_edge);
        if (ccw_neighbor->info().interior && !ccw_neighbor->info().processed) {
            int ccw_neighbor_edge = ccw_neighbor->index(face);
            // Validate neighbor edge index before recursion
            if (ccw_neighbor_edge >= 0 && ccw_neighbor_edge < 3) {
                std::list<Triangulation::Vertex_handle> ccw_vertices;
                get_face_boundary_vertices(ccw_neighbor, ccw_neighbor_edge, ccw_vertices);
                vertices.splice(vertices.end(), ccw_vertices);
            }
        }
        // FUNCTION_ID: prepair_func006 - END
    }
}
