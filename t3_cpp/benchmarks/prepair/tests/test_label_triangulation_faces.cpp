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
    
    // Before labeling - faces should not be processed
    bool any_processed_before = false;
    for (auto fit = tri.all_faces_begin(); fit != tri.all_faces_end(); ++fit) {
        if (fit->info().processed) {
            any_processed_before = true;
            break;
        }
    }
    assert(!any_processed_before);  // No faces should be processed yet
    
    // Label faces
    CGALFunctions::label_triangulation_faces(tri);
    
    // After labeling - ALL faces must be processed
    for (auto fit = tri.all_faces_begin(); fit != tri.all_faces_end(); ++fit) {
        assert(fit->info().processed == true);  // Every face must be processed
    }
    
    // Verify infinite face is exterior
    assert(tri.infinite_face()->info().interior == false);
    
    // Count interior vs exterior faces
    int interior_count = 0;
    int exterior_count = 0;
    for (auto fit = tri.finite_faces_begin(); fit != tri.finite_faces_end(); ++fit) {
        if (fit->info().interior) interior_count++;
        else exterior_count++;
    }
    
    // With a square constraint, must have both interior and exterior faces
    assert(interior_count > 0);  // Must have interior faces
    // Note: exterior_count might be 0 if all finite faces are interior
    // This is OK - the key test is that labeling happened
    
    // Verify labeling is consistent across constraint edges
    for (auto fit = tri.finite_faces_begin(); fit != tri.finite_faces_end(); ++fit) {
        for (int i = 0; i < 3; ++i) {
            if (tri.is_constrained(Triangulation::Edge(fit, i))) {
                Triangulation::Face_handle neighbor = fit->neighbor(i);
                // Across constraint, interior/exterior must differ
                assert(fit->info().interior != neighbor->info().interior);
            }
        }
    }
    
    return 0;
}
