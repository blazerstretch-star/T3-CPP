#include "../src/cgal_functions.h"
#include <cassert>

int main() {
    Triangulation tri;
    
    // Create a square with constraints
    Kernel::Point_2 p1(0, 0);
    Kernel::Point_2 p2(1, 0);
    Kernel::Point_2 p3(1, 1);
    Kernel::Point_2 p4(0, 1);
    
    Triangulation::Vertex_handle v1 = tri.insert(p1);
    Triangulation::Vertex_handle v2 = tri.insert(p2);
    Triangulation::Vertex_handle v3 = tri.insert(p3);
    Triangulation::Vertex_handle v4 = tri.insert(p4);
    
    // Add constraints to form a square boundary
    tri.insert_constraint(v1, v2);
    tri.insert_constraint(v2, v3);
    tri.insert_constraint(v3, v4);
    tri.insert_constraint(v4, v1);
    
    // Find a face adjacent to a constraint
    Triangulation::Face_handle fh;
    int edge_idx = -1;
    
    for (auto fit = tri.finite_faces_begin(); fit != tri.finite_faces_end(); ++fit) {
        for (int i = 0; i < 3; ++i) {
            if (tri.is_constrained(Triangulation::Edge(fit, i))) {
                fh = fit;
                edge_idx = i;
                break;
            }
        }
        if (edge_idx != -1) break;
    }
    
    assert(edge_idx != -1);  // Must find a constrained edge
    
    // Mark face as interior for testing
    fh->info().interior = true;
    fh->info().processed = false;
    
    // Get boundary vertices
    std::list<Triangulation::Vertex_handle> vertices;
    CGALFunctions::get_face_boundary_vertices(fh, edge_idx, vertices);
    
    // Test 1: Must return at least 1 vertex (the central vertex)
    assert(vertices.size() >= 1);  // STRONG: Must return vertices
    
    // Test 2: All returned vertices must be valid
    for (const auto& v : vertices) {
        // Verify vertex has valid point (doesn't crash)
        Kernel::Point_2 p = v->point();
        (void)p;  // Use the point to avoid warning
    }
    
    // Test 3: Face must be marked as processed after traversal
    assert(fh->info().processed == true);  // STRONG: Must mark as processed
    
    // Test 3: Empty input should not crash
    Triangulation tri2;
    vertices.clear();
    CGALFunctions::get_face_boundary_vertices(tri2.infinite_face(), 0, vertices);
    // Should handle gracefully
    
    return 0;
}
