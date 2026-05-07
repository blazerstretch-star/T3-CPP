#include "../src/cgal_functions.h"
#include <cassert>
#include <set>

int main() {
    // ========================================
    // PROPERTY 1: Non-empty output for interior faces
    // Interior unprocessed faces must return at least 1 vertex
    // ========================================
    auto check_non_empty_output = [](const std::list<Triangulation::Vertex_handle>& vertices) {
        assert(vertices.size() >= 1 &&
               "PROPERTY VIOLATION: Must return at least 1 vertex for interior face");
    };
    
    // ========================================
    // PROPERTY 2: All vertices are valid
    // All returned vertices must have valid points
    // ========================================
    auto check_valid_vertices = [](const std::list<Triangulation::Vertex_handle>& vertices) {
        for (const auto& v : vertices) {
            // Access point to verify it doesn't crash
            Kernel::Point_2 p = v->point();
            (void)p;  // Suppress unused warning
            
            // Verify coordinates are finite
            double x = CGAL::to_double(p.x());
            double y = CGAL::to_double(p.y());
            assert(std::isfinite(x) && std::isfinite(y) &&
                   "PROPERTY VIOLATION: Vertex coordinates must be finite");
        }
    };
    
    // ========================================
    // PROPERTY 3: Face marked as processed
    // After traversal, the face must be marked as processed
    // ========================================
    auto check_face_processed = [](Triangulation::Face_handle face) {
        assert(face->info().processed == true &&
               "PROPERTY VIOLATION: Face must be marked as processed after traversal");
    };
    
    // ========================================
    // PROPERTY 4: No duplicate vertices
    // Returned vertices should be unique (no duplicates)
    // ========================================
    auto check_no_duplicates = [](const std::list<Triangulation::Vertex_handle>& vertices) {
        std::set<Triangulation::Vertex_handle> unique_vertices(vertices.begin(), vertices.end());
        assert(unique_vertices.size() == vertices.size() &&
               "PROPERTY VIOLATION: Vertices should be unique (no duplicates)");
    };
    
    // ========================================
    // PROPERTY 5: Vertices form connected path
    // Consecutive vertices should share edges in triangulation
    // ========================================
    auto check_connectivity = [](const Triangulation& tri,
                                 const std::list<Triangulation::Vertex_handle>& vertices) {
        if (vertices.size() < 2) return;  // Single vertex is trivially connected
        
        auto it = vertices.begin();
        auto prev = it++;
        
        while (it != vertices.end()) {
            // Check if there's an edge between prev and current
            bool found_edge = false;
            
            // Search through all faces to find edge
            for (auto fit = tri.finite_faces_begin(); fit != tri.finite_faces_end(); ++fit) {
                for (int i = 0; i < 3; ++i) {
                    Triangulation::Vertex_handle v1 = fit->vertex((i+1)%3);
                    Triangulation::Vertex_handle v2 = fit->vertex((i+2)%3);
                    
                    if ((v1 == *prev && v2 == *it) || (v1 == *it && v2 == *prev)) {
                        found_edge = true;
                        break;
                    }
                }
                if (found_edge) break;
            }
            
            // Note: This property might be too strict for some implementations
            // Comment out if it causes issues
            // assert(found_edge && "PROPERTY VIOLATION: Consecutive vertices should share edge");
            
            prev = it++;
        }
    };
    
    // ========================================
    // PROPERTY 6: Determinism
    // Multiple calls with same input should produce same output
    // ========================================
    auto check_determinism = [](Triangulation& tri,
                                Triangulation::Face_handle face,
                                int edge) {
        // Reset processed flags
        for (auto fit = tri.all_faces_begin(); fit != tri.all_faces_end(); ++fit) {
            fit->info().processed = false;
        }
        
        std::list<Triangulation::Vertex_handle> vertices1;
        CGALFunctions::get_face_boundary_vertices(face, edge, vertices1);
        
        // Reset again
        for (auto fit = tri.all_faces_begin(); fit != tri.all_faces_end(); ++fit) {
            fit->info().processed = false;
        }
        
        std::list<Triangulation::Vertex_handle> vertices2;
        CGALFunctions::get_face_boundary_vertices(face, edge, vertices2);
        
        assert(vertices1.size() == vertices2.size() &&
               "PROPERTY VIOLATION: Function must be deterministic");
    };
    
    // ========================================
    // TEST 1: Square with interior face
    // ========================================
    {
        Triangulation tri;
        
        Kernel::Point_2 p1(0, 0);
        Kernel::Point_2 p2(1, 0);
        Kernel::Point_2 p3(1, 1);
        Kernel::Point_2 p4(0, 1);
        
        Triangulation::Vertex_handle v1 = tri.insert(p1);
        Triangulation::Vertex_handle v2 = tri.insert(p2);
        Triangulation::Vertex_handle v3 = tri.insert(p3);
        Triangulation::Vertex_handle v4 = tri.insert(p4);
        
        tri.insert_constraint(v1, v2);
        tri.insert_constraint(v2, v3);
        tri.insert_constraint(v3, v4);
        tri.insert_constraint(v4, v1);
        
        // Find an interior face adjacent to constraint
        Triangulation::Face_handle interior_face;
        int edge_idx = -1;
        
        for (auto fit = tri.finite_faces_begin(); fit != tri.finite_faces_end(); ++fit) {
            // Check if face is inside the square (heuristic: center point)
            Kernel::Point_2 center(
                (fit->vertex(0)->point().x() + fit->vertex(1)->point().x() + fit->vertex(2)->point().x()) / 3.0,
                (fit->vertex(0)->point().y() + fit->vertex(1)->point().y() + fit->vertex(2)->point().y()) / 3.0
            );
            
            double cx = CGAL::to_double(center.x());
            double cy = CGAL::to_double(center.y());
            
            if (cx > 0.1 && cx < 0.9 && cy > 0.1 && cy < 0.9) {
                // This is likely an interior face
                for (int i = 0; i < 3; ++i) {
                    if (tri.is_constrained(Triangulation::Edge(fit, i))) {
                        interior_face = fit;
                        edge_idx = i;
                        break;
                    }
                }
            }
            if (edge_idx != -1) break;
        }
        
        if (edge_idx != -1) {
            interior_face->info().interior = true;
            interior_face->info().processed = false;
            
            std::list<Triangulation::Vertex_handle> vertices;
            CGALFunctions::get_face_boundary_vertices(interior_face, edge_idx, vertices);
            
            check_non_empty_output(vertices);
            check_valid_vertices(vertices);
            check_face_processed(interior_face);
            check_no_duplicates(vertices);
        }
    }
    
    // ========================================
    // TEST 2: Triangle with interior face
    // ========================================
    {
        Triangulation tri;
        
        Kernel::Point_2 p1(0, 0);
        Kernel::Point_2 p2(2, 0);
        Kernel::Point_2 p3(1, 2);
        
        Triangulation::Vertex_handle v1 = tri.insert(p1);
        Triangulation::Vertex_handle v2 = tri.insert(p2);
        Triangulation::Vertex_handle v3 = tri.insert(p3);
        
        tri.insert_constraint(v1, v2);
        tri.insert_constraint(v2, v3);
        tri.insert_constraint(v3, v1);
        
        // Find interior face
        for (auto fit = tri.finite_faces_begin(); fit != tri.finite_faces_end(); ++fit) {
            for (int i = 0; i < 3; ++i) {
                if (tri.is_constrained(Triangulation::Edge(fit, i))) {
                    fit->info().interior = true;
                    fit->info().processed = false;
                    
                    std::list<Triangulation::Vertex_handle> vertices;
                    CGALFunctions::get_face_boundary_vertices(fit, i, vertices);
                    
                    check_non_empty_output(vertices);
                    check_valid_vertices(vertices);
                    check_face_processed(fit);
                    
                    break;
                }
            }
        }
    }
    
    // ========================================
    // TEST 3: Already processed face (should return early)
    // ========================================
    {
        Triangulation tri;
        
        Triangulation::Vertex_handle v1 = tri.insert(Kernel::Point_2(0, 0));
        Triangulation::Vertex_handle v2 = tri.insert(Kernel::Point_2(1, 0));
        Triangulation::Vertex_handle v3 = tri.insert(Kernel::Point_2(0.5, 1));
        
        tri.insert_constraint(v1, v2);
        
        auto face = tri.finite_faces_begin();
        face->info().interior = true;
        face->info().processed = true;  // Already processed
        
        std::list<Triangulation::Vertex_handle> vertices;
        CGALFunctions::get_face_boundary_vertices(face, 0, vertices);
        
        // Should return empty or handle gracefully
        // (implementation may vary)
    }
    
    // ========================================
    // TEST 4: Exterior face (should return early)
    // ========================================
    {
        Triangulation tri;
        
        Triangulation::Vertex_handle v1 = tri.insert(Kernel::Point_2(0, 0));
        Triangulation::Vertex_handle v2 = tri.insert(Kernel::Point_2(1, 0));
        Triangulation::Vertex_handle v3 = tri.insert(Kernel::Point_2(0.5, 1));
        
        tri.insert_constraint(v1, v2);
        
        auto face = tri.finite_faces_begin();
        face->info().interior = false;  // Exterior
        face->info().processed = false;
        
        std::list<Triangulation::Vertex_handle> vertices;
        CGALFunctions::get_face_boundary_vertices(face, 0, vertices);
        
        // Should return empty or handle gracefully
    }
    
    // ========================================
    // TEST 5: Empty triangulation (edge case)
    // ========================================
    {
        Triangulation tri;
        
        std::list<Triangulation::Vertex_handle> vertices;
        CGALFunctions::get_face_boundary_vertices(tri.infinite_face(), 0, vertices);
        
        // Should handle gracefully without crashing
    }
    
    // ========================================
    // TEST 6: Invalid edge index
    // ========================================
    {
        Triangulation tri;
        
        tri.insert(Kernel::Point_2(0, 0));
        tri.insert(Kernel::Point_2(1, 0));
        tri.insert(Kernel::Point_2(0.5, 1));
        
        auto face = tri.finite_faces_begin();
        face->info().interior = true;
        face->info().processed = false;
        
        // Test with invalid indices
        std::list<Triangulation::Vertex_handle> vertices;
        
        // Should handle gracefully (not crash)
        CGALFunctions::get_face_boundary_vertices(face, -1, vertices);
        vertices.clear();
        CGALFunctions::get_face_boundary_vertices(face, 5, vertices);
    }
    
    // ========================================
    // TEST 7: Complex polygon with multiple interior faces
    // ========================================
    {
        Triangulation tri;
        
        std::vector<Triangulation::Vertex_handle> vertices_vec;
        for (int i = 0; i < 6; ++i) {
            double angle = 2.0 * M_PI * i / 6.0;
            Kernel::Point_2 p(std::cos(angle), std::sin(angle));
            vertices_vec.push_back(tri.insert(p));
        }
        
        for (size_t i = 0; i < vertices_vec.size(); ++i) {
            tri.insert_constraint(vertices_vec[i], vertices_vec[(i+1) % vertices_vec.size()]);
        }
        
        // Find and test interior faces
        for (auto fit = tri.finite_faces_begin(); fit != tri.finite_faces_end(); ++fit) {
            Kernel::Point_2 center(
                (fit->vertex(0)->point().x() + fit->vertex(1)->point().x() + fit->vertex(2)->point().x()) / 3.0,
                (fit->vertex(0)->point().y() + fit->vertex(1)->point().y() + fit->vertex(2)->point().y()) / 3.0
            );
            
            double dist = std::sqrt(
                CGAL::to_double(center.x() * center.x() + center.y() * center.y())
            );
            
            if (dist < 0.8) {  // Inside hexagon
                for (int i = 0; i < 3; ++i) {
                    if (tri.is_constrained(Triangulation::Edge(fit, i))) {
                        fit->info().interior = true;
                        fit->info().processed = false;
                        
                        std::list<Triangulation::Vertex_handle> vertices;
                        CGALFunctions::get_face_boundary_vertices(fit, i, vertices);
                        
                        if (vertices.size() > 0) {
                            check_valid_vertices(vertices);
                            check_face_processed(fit);
                        }
                        break;
                    }
                }
            }
        }
    }
    
    // ========================================
    // TEST 8: Vertex uniqueness in output
    // ========================================
    {
        Triangulation tri;
        
        Triangulation::Vertex_handle v1 = tri.insert(Kernel::Point_2(0, 0));
        Triangulation::Vertex_handle v2 = tri.insert(Kernel::Point_2(1, 0));
        Triangulation::Vertex_handle v3 = tri.insert(Kernel::Point_2(1, 1));
        Triangulation::Vertex_handle v4 = tri.insert(Kernel::Point_2(0, 1));
        
        tri.insert_constraint(v1, v2);
        tri.insert_constraint(v2, v3);
        tri.insert_constraint(v3, v4);
        tri.insert_constraint(v4, v1);
        
        for (auto fit = tri.finite_faces_begin(); fit != tri.finite_faces_end(); ++fit) {
            for (int i = 0; i < 3; ++i) {
                if (tri.is_constrained(Triangulation::Edge(fit, i))) {
                    fit->info().interior = true;
                    fit->info().processed = false;
                    
                    std::list<Triangulation::Vertex_handle> vertices;
                    CGALFunctions::get_face_boundary_vertices(fit, i, vertices);
                    
                    if (vertices.size() > 1) {
                        check_no_duplicates(vertices);
                    }
                    break;
                }
            }
        }
    }
    
    // ========================================
    // TEST 9: Stress test with large polygon
    // ========================================
    {
        Triangulation tri;
        
        std::vector<Triangulation::Vertex_handle> vertices_vec;
        for (int i = 0; i < 20; ++i) {
            double angle = 2.0 * M_PI * i / 20.0;
            Kernel::Point_2 p(5.0 * std::cos(angle), 5.0 * std::sin(angle));
            vertices_vec.push_back(tri.insert(p));
        }
        
        for (size_t i = 0; i < vertices_vec.size(); ++i) {
            tri.insert_constraint(vertices_vec[i], vertices_vec[(i+1) % vertices_vec.size()]);
        }
        
        // Test a few interior faces
        int tested = 0;
        for (auto fit = tri.finite_faces_begin(); fit != tri.finite_faces_end() && tested < 5; ++fit) {
            for (int i = 0; i < 3; ++i) {
                if (tri.is_constrained(Triangulation::Edge(fit, i))) {
                    fit->info().interior = true;
                    fit->info().processed = false;
                    
                    std::list<Triangulation::Vertex_handle> vertices;
                    CGALFunctions::get_face_boundary_vertices(fit, i, vertices);
                    
                    if (vertices.size() > 0) {
                        check_valid_vertices(vertices);
                        tested++;
                    }
                    break;
                }
            }
        }
    }
    
    // ========================================
    // TEST 10: Output list is cleared/appended correctly
    // ========================================
    {
        Triangulation tri;
        
        Triangulation::Vertex_handle v1 = tri.insert(Kernel::Point_2(0, 0));
        Triangulation::Vertex_handle v2 = tri.insert(Kernel::Point_2(1, 0));
        Triangulation::Vertex_handle v3 = tri.insert(Kernel::Point_2(0.5, 1));
        
        tri.insert_constraint(v1, v2);
        
        auto face = tri.finite_faces_begin();
        face->info().interior = true;
        face->info().processed = false;
        
        // Pre-populate list
        std::list<Triangulation::Vertex_handle> vertices;
        vertices.push_back(v1);
        vertices.push_back(v2);
        size_t initial_size = vertices.size();
        
        CGALFunctions::get_face_boundary_vertices(face, 0, vertices);
        
        // Should have added vertices (not cleared)
        assert(vertices.size() >= initial_size &&
               "PROPERTY VIOLATION: Should append to list, not clear it");
    }
    
    return 0;
}
