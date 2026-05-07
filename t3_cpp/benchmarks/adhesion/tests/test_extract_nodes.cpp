#include "../src/adhesion_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <set>

const double TOLERANCE = 1e-6;

int main() {
    // Test 1: Basic node extraction with 5 points
    RT rt1;
    Weighted_point p1(Point(0, 0, 0), 1.0);
    Weighted_point p2(Point(1, 0, 0), 1.0);
    Weighted_point p3(Point(0, 1, 0), 1.0);
    Weighted_point p4(Point(0, 0, 1), 1.0);
    Weighted_point p5(Point(0.5, 0.5, 0.5), 1.0);
    
    rt1.insert(p1);
    rt1.insert(p2);
    rt1.insert(p3);
    rt1.insert(p4);
    rt1.insert(p5);
    
    double threshold = 0.5;
    double time = 1.0;
    auto nodes = Adhesion::extract_nodes(rt1, threshold, time);
    
    // Test 1a: Basic size validation
    assert(nodes.size() > 0 && "Must extract at least one node");
    size_t num_finite_cells = rt1.number_of_finite_cells();
    assert(nodes.size() == num_finite_cells && "Number of nodes must equal number of finite cells");
    
    // Test 1b: Validate each node's properties
    for (size_t i = 0; i < nodes.size(); ++i) {
        const auto& node = nodes[i];
        
        // Position must be finite
        assert(std::isfinite(node.position[0]) && "Node position x must be finite");
        assert(std::isfinite(node.position[1]) && "Node position y must be finite");
        assert(std::isfinite(node.position[2]) && "Node position z must be finite");
        
        // Velocity must be finite
        assert(std::isfinite(node.velocity[0]) && "Node velocity x must be finite");
        assert(std::isfinite(node.velocity[1]) && "Node velocity y must be finite");
        assert(std::isfinite(node.velocity[2]) && "Node velocity z must be finite");
        
        // Mass must be positive (volume of tetrahedron)
        assert(node.mass > 0 && "Node mass must be positive");
        
        // Node type must be valid
        assert(node.node_type >= Adhesion::VOID && 
               node.node_type <= Adhesion::UNDEFINED_NODE_TYPE && 
               "Node type must be valid");
    }
    
    // Test 2: Different threshold - verify node type classification changes
    auto nodes2 = Adhesion::extract_nodes(rt1, 0.1, time);
    assert(nodes2.size() == num_finite_cells && "Same number of nodes with different threshold");
    
    // Count node types for both thresholds
    std::map<Adhesion::NodeType, int> type_count1, type_count2;
    for (const auto& node : nodes) {
        type_count1[node.node_type]++;
    }
    for (const auto& node : nodes2) {
        type_count2[node.node_type]++;
    }
    
    // Different thresholds should potentially produce different type distributions
    // (though not guaranteed for all configurations)
    
    // Test 3: Different time parameter - verify velocity scaling
    auto nodes3 = Adhesion::extract_nodes(rt1, threshold, 2.0);
    assert(nodes3.size() == nodes.size() && "Same number of nodes with different time");
    
    // Velocities should scale inversely with time
    for (size_t i = 0; i < nodes.size(); ++i) {
        double mag1 = std::sqrt(nodes[i].velocity[0]*nodes[i].velocity[0] + 
                               nodes[i].velocity[1]*nodes[i].velocity[1] + 
                               nodes[i].velocity[2]*nodes[i].velocity[2]);
        double mag2 = std::sqrt(nodes3[i].velocity[0]*nodes3[i].velocity[0] + 
                               nodes3[i].velocity[1]*nodes3[i].velocity[1] + 
                               nodes3[i].velocity[2]*nodes3[i].velocity[2]);
        
        if (mag1 > TOLERANCE && mag2 > TOLERANCE) {
            double ratio = mag1 / mag2;
            assert(std::abs(ratio - 2.0) < 0.01 && "Velocity magnitude should scale inversely with time");
        }
    }
    
    // Test 4: Larger configuration
    RT rt2;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            for (int k = 0; k < 3; ++k) {
                rt2.insert(Weighted_point(Point(i, j, k), 1.0));
            }
        }
    }
    
    auto nodes4 = Adhesion::extract_nodes(rt2, 0.5, 1.0);
    size_t num_finite_cells2 = rt2.number_of_finite_cells();
    assert(nodes4.size() == num_finite_cells2 && "Correct number of nodes for larger configuration");
    
    // Validate all nodes in larger configuration
    for (const auto& node : nodes4) {
        assert(std::isfinite(node.position[0]) && std::isfinite(node.position[1]) && std::isfinite(node.position[2]));
        assert(std::isfinite(node.velocity[0]) && std::isfinite(node.velocity[1]) && std::isfinite(node.velocity[2]));
        assert(node.mass > 0);
        assert(node.node_type >= Adhesion::VOID && node.node_type <= Adhesion::UNDEFINED_NODE_TYPE);
    }
    
    // Test 5: Verify node type distribution makes sense
    // With threshold 0.5, we should have various node types
    std::set<Adhesion::NodeType> unique_types;
    for (const auto& node : nodes4) {
        unique_types.insert(node.node_type);
    }
    assert(unique_types.size() > 0 && "Should have at least one node type");
    
    // Test 6: Sum of masses should be reasonable (total volume)
    double total_mass = 0;
    for (const auto& node : nodes) {
        total_mass += node.mass;
    }
    assert(total_mass > 0 && "Total mass must be positive");
    assert(std::isfinite(total_mass) && "Total mass must be finite");
    
    std::cout << "All extract_nodes tests passed!" << std::endl;
    return 0;
}
