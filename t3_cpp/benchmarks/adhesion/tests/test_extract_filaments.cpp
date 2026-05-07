#include "../src/adhesion_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    // Test 1: Basic filament extraction
    RT rt1;
    Weighted_point p1(Point(0, 0, 0), 0);
    Weighted_point p2(Point(1, 0, 0), 0);
    Weighted_point p3(Point(0, 1, 0), 0);
    Weighted_point p4(Point(0, 0, 1), 0);
    Weighted_point p5(Point(1, 1, 1), 0);
    
    rt1.insert(p1);
    rt1.insert(p2);
    rt1.insert(p3);
    rt1.insert(p4);
    rt1.insert(p5);
    
    double threshold = 0.1;
    auto mesh = Adhesion::extract_filaments(rt1, threshold);
    
    // Test 1a: Mesh must have vertices
    assert(mesh.vertices.size() > 0 && "Mesh must have at least one vertex");
    
    // Test 1b: Mesh structure consistency
    assert(mesh.sizes.size() == mesh.info.size() && "Sizes and info arrays must have same length");
    assert(mesh.sizes.size() == mesh.size() && "Size method must match sizes array length");
    
    // Test 1c: Data array size must equal sum of all edge sizes
    size_t expected_data_size = 0;
    for (size_t i = 0; i < mesh.sizes.size(); ++i) {
        expected_data_size += mesh.sizes[i];
    }
    assert(mesh.data.size() == expected_data_size && "Data array size must equal sum of edge sizes");
    
    // Test 1d: All vertex indices in data must be valid
    for (size_t i = 0; i < mesh.data.size(); ++i) {
        assert(mesh.data[i] < mesh.vertices.size() && "Vertex index must be valid");
    }
    
    // Test 1e: All info values (face areas) must be positive
    for (size_t i = 0; i < mesh.info.size(); ++i) {
        assert(mesh.info[i] > 0 && "Face area must be positive");
        assert(std::isfinite(mesh.info[i]) && "Face area must be finite");
    }
    
    // Test 1f: Filament edges should have exactly 2 vertices
    for (size_t i = 0; i < mesh.sizes.size(); ++i) {
        assert(mesh.sizes[i] == 2 && "Filament edge must have exactly 2 vertices");
    }
    
    // Test 1g: Vertices must be finite
    for (size_t i = 0; i < mesh.vertices.size(); ++i) {
        assert(std::isfinite(mesh.vertices[i].x()) && "Vertex x must be finite");
        assert(std::isfinite(mesh.vertices[i].y()) && "Vertex y must be finite");
        assert(std::isfinite(mesh.vertices[i].z()) && "Vertex z must be finite");
    }
    
    // Test 1h: Each edge should connect two different vertices
    size_t offset = 0;
    for (size_t i = 0; i < mesh.sizes.size(); ++i) {
        assert(mesh.data[offset] != mesh.data[offset + 1] && "Edge vertices must be different");
        offset += mesh.sizes[i];
    }
    
    // Test 2: Different threshold - should affect number of filaments
    auto mesh2 = Adhesion::extract_filaments(rt1, 0.01);
    assert(mesh2.vertices.size() > 0 && "Mesh with smaller threshold must have vertices");
    assert(mesh2.sizes.size() == mesh2.info.size());
    
    // Validate all edges have 2 vertices
    for (size_t i = 0; i < mesh2.sizes.size(); ++i) {
        assert(mesh2.sizes[i] == 2);
    }
    
    // Smaller threshold should generally produce more or equal filaments
    assert(mesh2.size() >= mesh.size() && "Smaller threshold should produce more or equal filaments");
    
    // Test 3: Very large threshold - should produce fewer or no filaments
    auto mesh3 = Adhesion::extract_filaments(rt1, 10.0);
    assert(mesh3.sizes.size() == mesh3.info.size());
    assert(mesh3.size() <= mesh.size() && "Larger threshold should produce fewer or equal filaments");
    
    // Test 4: Larger configuration
    RT rt2;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            for (int k = 0; k < 4; ++k) {
                rt2.insert(Weighted_point(Point(i*0.5, j*0.5, k*0.5), 0));
            }
        }
    }
    
    auto mesh4 = Adhesion::extract_filaments(rt2, 0.1);
    assert(mesh4.vertices.size() > 0);
    assert(mesh4.sizes.size() == mesh4.info.size());
    
    // Validate structure
    size_t expected_data_size4 = 0;
    for (size_t i = 0; i < mesh4.sizes.size(); ++i) {
        expected_data_size4 += mesh4.sizes[i];
        assert(mesh4.sizes[i] == 2);
    }
    assert(mesh4.data.size() == expected_data_size4);
    
    // Validate all vertices and info
    for (size_t i = 0; i < mesh4.vertices.size(); ++i) {
        assert(std::isfinite(mesh4.vertices[i].x()));
        assert(std::isfinite(mesh4.vertices[i].y()));
        assert(std::isfinite(mesh4.vertices[i].z()));
    }
    for (size_t i = 0; i < mesh4.info.size(); ++i) {
        assert(mesh4.info[i] > 0);
        assert(std::isfinite(mesh4.info[i]));
    }
    for (size_t i = 0; i < mesh4.data.size(); ++i) {
        assert(mesh4.data[i] < mesh4.vertices.size());
    }
    
    // Validate edges connect different vertices
    offset = 0;
    for (size_t i = 0; i < mesh4.sizes.size(); ++i) {
        assert(mesh4.data[offset] != mesh4.data[offset + 1]);
        offset += mesh4.sizes[i];
    }
    
    // Test 5: Verify vertices are within reasonable bounds
    for (size_t i = 0; i < mesh.vertices.size(); ++i) {
        // Vertices should be within or near the bounding box of input points
        assert(mesh.vertices[i].x() >= -1.0 && mesh.vertices[i].x() <= 2.0);
        assert(mesh.vertices[i].y() >= -1.0 && mesh.vertices[i].y() <= 2.0);
        assert(mesh.vertices[i].z() >= -1.0 && mesh.vertices[i].z() <= 2.0);
    }
    
    // Test 6: Number of unique vertices should be reasonable
    // For filaments, we expect fewer unique vertices than total edges
    assert(mesh.vertices.size() <= mesh.size() + 1 && 
           "Number of vertices should be reasonable for edge mesh");
    
    std::cout << "All extract_filaments tests passed!" << std::endl;
    return 0;
}
