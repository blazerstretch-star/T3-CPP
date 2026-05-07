#include "../src/cgal_functions.h"
#include <cassert>

int main() {
    Triangulation tri;
    Kernel::Plane_3 plane(Kernel::Point_3(0,0,0), Kernel::Vector_3(0,0,1));
    
    // Test 1: Insert square polygon
    std::vector<std::vector<double>> coords = {{0,0}, {1,0}, {1,1}, {0,1}};
    Triangulation::Face_handle fh = tri.infinite_face();
    
    int vertices_before = tri.number_of_vertices();
    
    CGALFunctions::insert_polygon_constraints(coords, tri, plane, fh);
    
    // Verify vertices were added
    assert(tri.number_of_vertices() == vertices_before + 4);  // Exactly 4 new vertices
    
    // Verify constraints were added (square has 4 edges)
    int constraint_count = 0;
    for (auto eit = tri.constrained_edges_begin(); eit != tri.constrained_edges_end(); ++eit) {
        constraint_count++;
    }
    assert(constraint_count == 4);  // Exactly 4 constraints
    
    // Test 2: Insert triangle
    Triangulation tri2;
    coords = {{0,0}, {2,0}, {1,2}};
    fh = tri2.infinite_face();
    
    CGALFunctions::insert_polygon_constraints(coords, tri2, plane, fh);
    
    assert(tri2.number_of_vertices() == 3);  // 3 vertices
    
    constraint_count = 0;
    for (auto eit = tri2.constrained_edges_begin(); eit != tri2.constrained_edges_end(); ++eit) {
        constraint_count++;
    }
    assert(constraint_count == 3);  // 3 constraints for triangle
    
    // Test 3: Empty polygon should not crash
    Triangulation tri3;
    coords = {};
    fh = tri3.infinite_face();
    
    CGALFunctions::insert_polygon_constraints(coords, tri3, plane, fh);
    assert(tri3.number_of_vertices() == 0);  // No vertices added
    
    return 0;
}
