#include "../src/cgal_functions.h"
#include <cassert>

int main() {
    // ========================================
    // PROPERTY 1: Boolean correctness
    // Constrained edges must return true, unconstrained must return false
    // ========================================
    auto check_constraint_detection = [](const Triangulation& tri,
                                         Triangulation::Face_handle face,
                                         int edge_idx,
                                         bool expected) {
        bool result = CGALFunctions::is_triangulation_edge_constrained(tri, face, edge_idx);
        assert(result == expected &&
               "PROPERTY VIOLATION: Constraint detection must match actual constraint status");
    };
    
    // ========================================
    // PROPERTY 2: Symmetry
    // If edge (face, i) is constrained, then edge (neighbor, j) is also constrained
    // ========================================
    auto check_symmetry = [](const Triangulation& tri,
                            Triangulation::Face_handle face,
                            int edge_idx) {
        bool is_constrained = CGALFunctions::is_triangulation_edge_constrained(tri, face, edge_idx);
        
        Triangulation::Face_handle neighbor = face->neighbor(edge_idx);
        int neighbor_edge_idx = neighbor->index(face);
        bool neighbor_constrained = CGALFunctions::is_triangulation_edge_constrained(tri, neighbor, neighbor_edge_idx);
        
        assert(is_constrained == neighbor_constrained &&
               "PROPERTY VIOLATION: Constraint status must be symmetric across edge");
    };
    
    // ========================================
    // PROPERTY 3: Consistency with CGAL
    // Result must match CGAL's is_constrained method
    // ========================================
    auto check_cgal_consistency = [](const Triangulation& tri,
                                     Triangulation::Face_handle face,
                                     int edge_idx) {
        bool our_result = CGALFunctions::is_triangulation_edge_constrained(tri, face, edge_idx);
        bool cgal_result = tri.is_constrained(Triangulation::Edge(face, edge_idx));
        
        assert(our_result == cgal_result &&
               "PROPERTY VIOLATION: Must match CGAL's is_constrained result");
    };
    
    // ========================================
    // TEST 1: Constrained edge must return true
    // ========================================
    {
        Triangulation tri;
        
        Kernel::Point_2 p1(0, 0);
        Kernel::Point_2 p2(1, 0);
        Kernel::Point_2 p3(0.5, 1);
        
        Triangulation::Vertex_handle v1 = tri.insert(p1);
        Triangulation::Vertex_handle v2 = tri.insert(p2);
        Triangulation::Vertex_handle v3 = tri.insert(p3);
        
        // Add constraint between v1 and v2
        tri.insert_constraint(v1, v2);
        
        // Find the constrained edge
        Triangulation::Face_handle constrained_face;
        int constrained_edge_idx = -1;
        
        for (auto fit = tri.finite_faces_begin(); fit != tri.finite_faces_end(); ++fit) {
            for (int i = 0; i < 3; ++i) {
                if ((fit->vertex((i+1)%3) == v1 && fit->vertex((i+2)%3) == v2) ||
                    (fit->vertex((i+1)%3) == v2 && fit->vertex((i+2)%3) == v1)) {
                    constrained_face = fit;
                    constrained_edge_idx = i;
                    break;
                }
            }
            if (constrained_edge_idx != -1) break;
        }
        
        assert(constrained_edge_idx != -1 && "Must find the constrained edge");
        
        check_constraint_detection(tri, constrained_face, constrained_edge_idx, true);
        check_symmetry(tri, constrained_face, constrained_edge_idx);
        check_cgal_consistency(tri, constrained_face, constrained_edge_idx);
    }
    
    // ========================================
    // TEST 2: Unconstrained edges must return false
    // ========================================
    {
        Triangulation tri;
        
        Kernel::Point_2 p1(0, 0);
        Kernel::Point_2 p2(1, 0);
        Kernel::Point_2 p3(0.5, 1);
        
        Triangulation::Vertex_handle v1 = tri.insert(p1);
        Triangulation::Vertex_handle v2 = tri.insert(p2);
        Triangulation::Vertex_handle v3 = tri.insert(p3);
        
        // Add constraint between v1 and v2 only
        tri.insert_constraint(v1, v2);
        
        // Find an unconstrained edge
        for (auto fit = tri.finite_faces_begin(); fit != tri.finite_faces_end(); ++fit) {
            for (int i = 0; i < 3; ++i) {
                if (!tri.is_constrained(Triangulation::Edge(fit, i))) {
                    check_constraint_detection(tri, fit, i, false);
                    check_symmetry(tri, fit, i);
                    check_cgal_consistency(tri, fit, i);
                }
            }
        }
    }
    
    // ========================================
    // TEST 3: Invalid edge index must return false
    // ========================================
    {
        Triangulation tri;
        
        Triangulation::Vertex_handle v1 = tri.insert(Kernel::Point_2(0, 0));
        Triangulation::Vertex_handle v2 = tri.insert(Kernel::Point_2(1, 0));
        Triangulation::Vertex_handle v3 = tri.insert(Kernel::Point_2(0.5, 1));
        
        tri.insert_constraint(v1, v2);
        
        auto face = tri.finite_faces_begin();
        
        // Test invalid indices
        assert(CGALFunctions::is_triangulation_edge_constrained(tri, face, -1) == false &&
               "PROPERTY VIOLATION: Negative index must return false");
        assert(CGALFunctions::is_triangulation_edge_constrained(tri, face, 3) == false &&
               "PROPERTY VIOLATION: Index >= 3 must return false");
        assert(CGALFunctions::is_triangulation_edge_constrained(tri, face, 100) == false &&
               "PROPERTY VIOLATION: Large invalid index must return false");
    }
    
    // ========================================
    // TEST 4: All edges in constrained polygon
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
        
        // Count constrained edges
        int constrained_count = 0;
        int unconstrained_count = 0;
        
        for (auto fit = tri.finite_faces_begin(); fit != tri.finite_faces_end(); ++fit) {
            for (int i = 0; i < 3; ++i) {
                bool is_constrained = CGALFunctions::is_triangulation_edge_constrained(tri, fit, i);
                
                if (is_constrained) {
                    constrained_count++;
                    check_symmetry(tri, fit, i);
                    check_cgal_consistency(tri, fit, i);
                } else {
                    unconstrained_count++;
                    check_symmetry(tri, fit, i);
                    check_cgal_consistency(tri, fit, i);
                }
            }
        }
        
        assert(constrained_count > 0 &&
               "PROPERTY VIOLATION: Must have some constrained edges");
        assert(unconstrained_count > 0 &&
               "PROPERTY VIOLATION: Must have some unconstrained edges");
    }
    
    // ========================================
    // TEST 5: No constraints (all false)
    // ========================================
    {
        Triangulation tri;
        
        tri.insert(Kernel::Point_2(0, 0));
        tri.insert(Kernel::Point_2(1, 0));
        tri.insert(Kernel::Point_2(0.5, 1));
        
        // No constraints added
        for (auto fit = tri.finite_faces_begin(); fit != tri.finite_faces_end(); ++fit) {
            for (int i = 0; i < 3; ++i) {
                check_constraint_detection(tri, fit, i, false);
                check_cgal_consistency(tri, fit, i);
            }
        }
    }
    
    // ========================================
    // TEST 6: Multiple constraints
    // ========================================
    {
        Triangulation tri;
        
        std::vector<Triangulation::Vertex_handle> vertices;
        for (int i = 0; i < 6; ++i) {
            double angle = 2.0 * M_PI * i / 6.0;
            Kernel::Point_2 p(std::cos(angle), std::sin(angle));
            vertices.push_back(tri.insert(p));
        }
        
        // Add constraints for hexagon
        for (size_t i = 0; i < vertices.size(); ++i) {
            tri.insert_constraint(vertices[i], vertices[(i+1) % vertices.size()]);
        }
        
        // Verify all edges
        for (auto fit = tri.finite_faces_begin(); fit != tri.finite_faces_end(); ++fit) {
            for (int i = 0; i < 3; ++i) {
                check_symmetry(tri, fit, i);
                check_cgal_consistency(tri, fit, i);
            }
        }
    }
    
    // ========================================
    // TEST 7: Determinism
    // Multiple calls should return same result
    // ========================================
    {
        Triangulation tri;
        
        Triangulation::Vertex_handle v1 = tri.insert(Kernel::Point_2(0, 0));
        Triangulation::Vertex_handle v2 = tri.insert(Kernel::Point_2(1, 0));
        Triangulation::Vertex_handle v3 = tri.insert(Kernel::Point_2(0.5, 1));
        
        tri.insert_constraint(v1, v2);
        
        for (auto fit = tri.finite_faces_begin(); fit != tri.finite_faces_end(); ++fit) {
            for (int i = 0; i < 3; ++i) {
                bool result1 = CGALFunctions::is_triangulation_edge_constrained(tri, fit, i);
                bool result2 = CGALFunctions::is_triangulation_edge_constrained(tri, fit, i);
                bool result3 = CGALFunctions::is_triangulation_edge_constrained(tri, fit, i);
                
                assert(result1 == result2 && result2 == result3 &&
                       "PROPERTY VIOLATION: Function must be deterministic");
            }
        }
    }
    
    // ========================================
    // TEST 8: Boundary edges (infinite face)
    // ========================================
    {
        Triangulation tri;
        
        tri.insert(Kernel::Point_2(0, 0));
        tri.insert(Kernel::Point_2(1, 0));
        tri.insert(Kernel::Point_2(0.5, 1));
        
        // Test edges of infinite face
        Triangulation::Face_handle inf_face = tri.infinite_face();
        for (int i = 0; i < 3; ++i) {
            // Should handle gracefully (return false or not crash)
            bool result = CGALFunctions::is_triangulation_edge_constrained(tri, inf_face, i);
            (void)result;  // Just ensure it doesn't crash
        }
    }
    
    // ========================================
    // TEST 9: Stress test with many constraints
    // ========================================
    {
        Triangulation tri;
        
        std::vector<Triangulation::Vertex_handle> vertices;
        for (int i = 0; i < 20; ++i) {
            double angle = 2.0 * M_PI * i / 20.0;
            Kernel::Point_2 p(5.0 * std::cos(angle), 5.0 * std::sin(angle));
            vertices.push_back(tri.insert(p));
        }
        
        for (size_t i = 0; i < vertices.size(); ++i) {
            tri.insert_constraint(vertices[i], vertices[(i+1) % vertices.size()]);
        }
        
        // Verify all edges
        int total_edges = 0;
        for (auto fit = tri.finite_faces_begin(); fit != tri.finite_faces_end(); ++fit) {
            for (int i = 0; i < 3; ++i) {
                check_cgal_consistency(tri, fit, i);
                total_edges++;
            }
        }
        
        assert(total_edges > 0 &&
               "PROPERTY VIOLATION: Must have edges to test");
    }
    
    // ========================================
    // TEST 10: Edge case - valid indices only
    // ========================================
    {
        Triangulation tri;
        
        tri.insert(Kernel::Point_2(0, 0));
        tri.insert(Kernel::Point_2(1, 0));
        tri.insert(Kernel::Point_2(0.5, 1));
        
        auto face = tri.finite_faces_begin();
        
        // Only indices 0, 1, 2 are valid
        for (int i = 0; i < 3; ++i) {
            bool result = CGALFunctions::is_triangulation_edge_constrained(tri, face, i);
            // Should not crash and should match CGAL
            check_cgal_consistency(tri, face, i);
        }
    }
    
    return 0;
}
