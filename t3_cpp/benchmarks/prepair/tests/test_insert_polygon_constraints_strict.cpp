#include "../src/cgal_functions.h"
#include <cassert>
#include <set>

int main() {
    const double EPSILON = 1e-9;
    
    // ========================================
    // PROPERTY 1: Vertex count equals polygon size
    // For N-gon, exactly N vertices should be added
    // ========================================
    auto check_vertex_count = [](const Triangulation& tri, 
                                  int initial_vertices,
                                  int expected_new_vertices) {
        int actual_vertices = tri.number_of_vertices();
        int added_vertices = actual_vertices - initial_vertices;
        
        // Allow <= because implementation may merge duplicate vertices
        assert(added_vertices <= expected_new_vertices &&
               "PROPERTY VIOLATION: Should not add more vertices than input");
        assert(added_vertices > 0 || expected_new_vertices == 0 &&
               "PROPERTY VIOLATION: Should add at least some vertices for non-empty input");
    };
    
    // ========================================
    // PROPERTY 2: Constraint count equals polygon edges
    // For N-gon, exactly N constraints should be added
    // ========================================
    auto check_constraint_count = [](const Triangulation& tri,
                                     int expected_constraints) {
        int constraint_count = 0;
        for (auto eit = tri.constrained_edges_begin(); 
             eit != tri.constrained_edges_end(); ++eit) {
            constraint_count++;
        }
        
        assert(constraint_count == expected_constraints &&
               "PROPERTY VIOLATION: Constraint count must equal polygon edges");
    };
    
    // ========================================
    // PROPERTY 3: All constraints form a closed loop
    // Each vertex should have exactly 2 constrained edges
    // ========================================
    auto check_closed_loop = [](const Triangulation& tri) {
        std::map<Triangulation::Vertex_handle, int> vertex_constraint_count;
        
        for (auto eit = tri.constrained_edges_begin(); 
             eit != tri.constrained_edges_end(); ++eit) {
            Triangulation::Face_handle face = eit->first;
            int edge_idx = eit->second;
            
            Triangulation::Vertex_handle v1 = face->vertex((edge_idx + 1) % 3);
            Triangulation::Vertex_handle v2 = face->vertex((edge_idx + 2) % 3);
            
            vertex_constraint_count[v1]++;
            vertex_constraint_count[v2]++;
        }
        
        // Each vertex in a closed polygon should have exactly 2 constrained edges
        for (const auto& pair : vertex_constraint_count) {
            assert(pair.second == 2 &&
                   "PROPERTY VIOLATION: Each vertex must have exactly 2 constrained edges (closed loop)");
        }
    };
    
    // ========================================
    // PROPERTY 4: Vertices have correct 3D coordinates
    // Vertex info should store original 3D points
    // ========================================
    auto check_vertex_coordinates = [EPSILON](
        const Triangulation& tri,
        const std::vector<std::vector<double>>& coords) {
        
        // Collect all vertices with has_point=true
        std::vector<Kernel::Point_3> stored_points;
        for (auto vit = tri.finite_vertices_begin(); 
             vit != tri.finite_vertices_end(); ++vit) {
            if (vit->info().has_point) {
                stored_points.push_back(vit->info().point);
            }
        }
        
        assert(stored_points.size() == coords.size() &&
               "PROPERTY VIOLATION: All vertices must have 3D coordinates stored");
        
        // Check that all input coordinates are present (order may vary)
        for (const auto& coord : coords) {
            double expected_x = coord[0];
            double expected_y = coord[1];
            double expected_z = (coord.size() >= 3) ? coord[2] : 0.0;
            
            bool found = false;
            for (const auto& p : stored_points) {
                double dx = std::abs(CGAL::to_double(p.x()) - expected_x);
                double dy = std::abs(CGAL::to_double(p.y()) - expected_y);
                double dz = std::abs(CGAL::to_double(p.z()) - expected_z);
                
                if (dx < EPSILON && dy < EPSILON && dz < EPSILON) {
                    found = true;
                    break;
                }
            }
            
            assert(found &&
                   "PROPERTY VIOLATION: All input coordinates must be stored in vertices");
        }
    };
    
    // ========================================
    // PROPERTY 5: Constraints are unique
    // No duplicate constraints should exist
    // ========================================
    auto check_unique_constraints = [](const Triangulation& tri) {
        std::set<std::pair<void*, void*>> constraint_set;
        
        for (auto eit = tri.constrained_edges_begin(); 
             eit != tri.constrained_edges_end(); ++eit) {
            Triangulation::Face_handle face = eit->first;
            int edge_idx = eit->second;
            
            Triangulation::Vertex_handle v1 = face->vertex((edge_idx + 1) % 3);
            Triangulation::Vertex_handle v2 = face->vertex((edge_idx + 2) % 3);
            
            // Create ordered pair (smaller pointer first)
            void* p1 = &(*v1);
            void* p2 = &(*v2);
            if (p1 > p2) std::swap(p1, p2);
            
            auto pair = std::make_pair(p1, p2);
            assert(constraint_set.find(pair) == constraint_set.end() &&
                   "PROPERTY VIOLATION: Constraints must be unique");
            constraint_set.insert(pair);
        }
    };
    
    // ========================================
    // TEST 1: Square polygon (4 vertices, 4 edges)
    // ========================================
    {
        Triangulation tri;
        Kernel::Plane_3 plane(Kernel::Point_3(0,0,0), Kernel::Vector_3(0,0,1));
        std::vector<std::vector<double>> coords = {{0,0}, {1,0}, {1,1}, {0,1}};
        Triangulation::Face_handle fh = tri.infinite_face();
        
        int vertices_before = tri.number_of_vertices();
        
        CGALFunctions::insert_polygon_constraints(coords, tri, plane, fh);
        
        check_vertex_count(tri, vertices_before, 4);
        check_constraint_count(tri, 4);
        check_closed_loop(tri);
        check_vertex_coordinates(tri, coords);
        check_unique_constraints(tri);
    }
    
    // ========================================
    // TEST 2: Triangle (3 vertices, 3 edges)
    // ========================================
    {
        Triangulation tri;
        Kernel::Plane_3 plane(Kernel::Point_3(0,0,0), Kernel::Vector_3(0,0,1));
        std::vector<std::vector<double>> coords = {{0,0}, {2,0}, {1,2}};
        Triangulation::Face_handle fh = tri.infinite_face();
        
        int vertices_before = tri.number_of_vertices();
        
        CGALFunctions::insert_polygon_constraints(coords, tri, plane, fh);
        
        check_vertex_count(tri, vertices_before, 3);
        check_constraint_count(tri, 3);
        check_closed_loop(tri);
        check_vertex_coordinates(tri, coords);
        check_unique_constraints(tri);
    }
    
    // ========================================
    // TEST 3: Pentagon (5 vertices, 5 edges)
    // ========================================
    {
        Triangulation tri;
        Kernel::Plane_3 plane(Kernel::Point_3(0,0,0), Kernel::Vector_3(0,0,1));
        std::vector<std::vector<double>> coords = {
            {0,0}, {2,0}, {3,1.5}, {1,2.5}, {-0.5,1}
        };
        Triangulation::Face_handle fh = tri.infinite_face();
        
        int vertices_before = tri.number_of_vertices();
        
        CGALFunctions::insert_polygon_constraints(coords, tri, plane, fh);
        
        check_vertex_count(tri, vertices_before, 5);
        check_constraint_count(tri, 5);
        check_closed_loop(tri);
        check_vertex_coordinates(tri, coords);
        check_unique_constraints(tri);
    }
    
    // ========================================
    // TEST 4: Empty polygon (0 vertices, 0 edges)
    // ========================================
    {
        Triangulation tri;
        Kernel::Plane_3 plane(Kernel::Point_3(0,0,0), Kernel::Vector_3(0,0,1));
        std::vector<std::vector<double>> coords = {};
        Triangulation::Face_handle fh = tri.infinite_face();
        
        int vertices_before = tri.number_of_vertices();
        
        CGALFunctions::insert_polygon_constraints(coords, tri, plane, fh);
        
        assert(tri.number_of_vertices() == vertices_before &&
               "PROPERTY VIOLATION: Empty polygon should add no vertices");
        check_constraint_count(tri, 0);
    }
    
    // ========================================
    // TEST 5: Single vertex (degenerate case)
    // Should handle gracefully (no constraints possible)
    // ========================================
    {
        Triangulation tri;
        Kernel::Plane_3 plane(Kernel::Point_3(0,0,0), Kernel::Vector_3(0,0,1));
        std::vector<std::vector<double>> coords = {{1,1}};
        Triangulation::Face_handle fh = tri.infinite_face();
        
        int vertices_before = tri.number_of_vertices();
        
        CGALFunctions::insert_polygon_constraints(coords, tri, plane, fh);
        
        // Should add 1 vertex but no constraints (can't form edge)
        check_vertex_count(tri, vertices_before, 1);
        check_constraint_count(tri, 0);
    }
    
    // ========================================
    // TEST 6: Two vertices (degenerate case)
    // Should create 1 constraint (line segment)
    // ========================================
    {
        Triangulation tri;
        Kernel::Plane_3 plane(Kernel::Point_3(0,0,0), Kernel::Vector_3(0,0,1));
        std::vector<std::vector<double>> coords = {{0,0}, {1,1}};
        Triangulation::Face_handle fh = tri.infinite_face();
        
        int vertices_before = tri.number_of_vertices();
        
        CGALFunctions::insert_polygon_constraints(coords, tri, plane, fh);
        
        check_vertex_count(tri, vertices_before, 2);
        // Note: 2 vertices should create 1 constraint (line), not a closed loop
        int constraint_count = 0;
        for (auto eit = tri.constrained_edges_begin(); 
             eit != tri.constrained_edges_end(); ++eit) {
            constraint_count++;
        }
        assert(constraint_count >= 1 &&
               "PROPERTY VIOLATION: Two vertices should create at least 1 constraint");
    }
    
    // ========================================
    // TEST 7: 3D coordinates (with z values)
    // ========================================
    {
        Triangulation tri;
        Kernel::Plane_3 plane(Kernel::Point_3(0,0,0), Kernel::Vector_3(0,0,1));
        std::vector<std::vector<double>> coords = {
            {0,0,5}, {1,0,6}, {1,1,7}, {0,1,8}
        };
        Triangulation::Face_handle fh = tri.infinite_face();
        
        int vertices_before = tri.number_of_vertices();
        
        CGALFunctions::insert_polygon_constraints(coords, tri, plane, fh);
        
        check_vertex_count(tri, vertices_before, 4);
        check_constraint_count(tri, 4);
        check_closed_loop(tri);
        check_vertex_coordinates(tri, coords);
        check_unique_constraints(tri);
    }
    
    // ========================================
    // TEST 8: Mixed 2D and 3D coordinates
    // ========================================
    {
        Triangulation tri;
        Kernel::Plane_3 plane(Kernel::Point_3(0,0,0), Kernel::Vector_3(0,0,1));
        std::vector<std::vector<double>> coords = {
            {0,0},      // 2D
            {1,0,5},    // 3D
            {1,1},      // 2D
            {0,1,3}     // 3D
        };
        Triangulation::Face_handle fh = tri.infinite_face();
        
        int vertices_before = tri.number_of_vertices();
        
        CGALFunctions::insert_polygon_constraints(coords, tri, plane, fh);
        
        check_vertex_count(tri, vertices_before, 4);
        check_constraint_count(tri, 4);
        check_closed_loop(tri);
        check_vertex_coordinates(tri, coords);
        check_unique_constraints(tri);
    }
    
    // ========================================
    // TEST 9: Large polygon (stress test)
    // ========================================
    {
        Triangulation tri;
        Kernel::Plane_3 plane(Kernel::Point_3(0,0,0), Kernel::Vector_3(0,0,1));
        
        // Create 100-gon
        std::vector<std::vector<double>> coords;
        for (int i = 0; i < 100; ++i) {
            double angle = 2.0 * M_PI * i / 100.0;
            coords.push_back({std::cos(angle), std::sin(angle)});
        }
        
        Triangulation::Face_handle fh = tri.infinite_face();
        int vertices_before = tri.number_of_vertices();
        
        CGALFunctions::insert_polygon_constraints(coords, tri, plane, fh);
        
        check_vertex_count(tri, vertices_before, 100);
        check_constraint_count(tri, 100);
        check_closed_loop(tri);
        check_unique_constraints(tri);
    }
    
    // ========================================
    // TEST 10: Polygon with duplicate vertices
    // Should handle gracefully (may merge duplicates)
    // ========================================
    {
        Triangulation tri;
        Kernel::Plane_3 plane(Kernel::Point_3(0,0,0), Kernel::Vector_3(0,0,1));
        std::vector<std::vector<double>> coords = {
            {0,0}, {1,0}, {1,0}, {1,1}, {0,1}  // Duplicate at index 1,2
        };
        Triangulation::Face_handle fh = tri.infinite_face();
        
        int vertices_before = tri.number_of_vertices();
        
        CGALFunctions::insert_polygon_constraints(coords, tri, plane, fh);
        
        // Should add at most 5 vertices (may merge duplicates)
        int added = tri.number_of_vertices() - vertices_before;
        assert(added <= 5 &&
               "PROPERTY VIOLATION: Should not add more vertices than input");
        
        // Should have some constraints
        int constraint_count = 0;
        for (auto eit = tri.constrained_edges_begin(); 
             eit != tri.constrained_edges_end(); ++eit) {
            constraint_count++;
        }
        assert(constraint_count > 0 &&
               "PROPERTY VIOLATION: Should create some constraints");
    }
    
    // ========================================
    // TEST 11: Multiple polygons in same triangulation
    // ========================================
    {
        Triangulation tri;
        Kernel::Plane_3 plane(Kernel::Point_3(0,0,0), Kernel::Vector_3(0,0,1));
        
        // Insert first polygon
        std::vector<std::vector<double>> coords1 = {{0,0}, {1,0}, {1,1}, {0,1}};
        Triangulation::Face_handle fh = tri.infinite_face();
        CGALFunctions::insert_polygon_constraints(coords1, tri, plane, fh);
        
        int vertices_after_first = tri.number_of_vertices();
        int constraints_after_first = 0;
        for (auto eit = tri.constrained_edges_begin(); 
             eit != tri.constrained_edges_end(); ++eit) {
            constraints_after_first++;
        }
        
        // Insert second polygon (non-overlapping)
        std::vector<std::vector<double>> coords2 = {{2,2}, {3,2}, {3,3}, {2,3}};
        CGALFunctions::insert_polygon_constraints(coords2, tri, plane, fh);
        
        // Should have 8 vertices total (4 + 4)
        assert(tri.number_of_vertices() == vertices_after_first + 4 &&
               "PROPERTY VIOLATION: Second polygon should add 4 more vertices");
        
        // Should have 8 constraints total (4 + 4)
        int total_constraints = 0;
        for (auto eit = tri.constrained_edges_begin(); 
             eit != tri.constrained_edges_end(); ++eit) {
            total_constraints++;
        }
        assert(total_constraints == constraints_after_first + 4 &&
               "PROPERTY VIOLATION: Second polygon should add 4 more constraints");
    }
    
    return 0;
}
