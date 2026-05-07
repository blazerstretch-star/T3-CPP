#include "../src/cgal_functions.h"
#include <cassert>

int main() {
    Triangulation tri;
    
    // Create triangulation with constraint
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
    
    assert(constrained_edge_idx != -1);  // Must find the edge
    
    // Test 1: Constrained edge must return true
    bool is_constrained = CGALFunctions::is_triangulation_edge_constrained(tri, constrained_face, constrained_edge_idx);
    assert(is_constrained == true);  // MUST be true for constrained edge
    
    // Test 2: Other edges in same face must return false
    int unconstrained_edge1 = (constrained_edge_idx + 1) % 3;
    int unconstrained_edge2 = (constrained_edge_idx + 2) % 3;
    
    bool is_unconstrained1 = CGALFunctions::is_triangulation_edge_constrained(tri, constrained_face, unconstrained_edge1);
    bool is_unconstrained2 = CGALFunctions::is_triangulation_edge_constrained(tri, constrained_face, unconstrained_edge2);
    
    assert(is_unconstrained1 == false);  // MUST be false
    assert(is_unconstrained2 == false);  // MUST be false
    
    // Test 3: Invalid edge index should return false
    bool invalid = CGALFunctions::is_triangulation_edge_constrained(tri, constrained_face, 5);
    assert(invalid == false);  // Out of range should be false
    
    return 0;
}
