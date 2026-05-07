#include "../src/route_planning_functions.h"
#include <cassert>
#include <iostream>

using namespace RoutePlanning;

int main() {
    // Test 1: Simple cycle
    Graph cycle;
    for(int i = 0; i < 5; i++) {
        cycle.nodes.emplace_back(std::cos(i * 2 * M_PI / 5), std::sin(i * 2 * M_PI / 5), i);
    }
    
    for(int i = 0; i < 5; i++) {
        cycle.nodes[i].neighbors.push_back(&cycle.nodes[(i+1) % 5]);
        cycle.nodes[i].neighbors.push_back(&cycle.nodes[(i+4) % 5]);
    }
    
    std::vector<Node> path = a_star_search(cycle, 0, 2);
    assert(!path.empty() && path.size() <= 3);
    
    // Test 2: Multiple equal-cost paths
    Graph diamond;
    diamond.nodes.emplace_back(0.0f, 0.0f, 0); // Start
    diamond.nodes.emplace_back(1.0f, 1.0f, 1); // Top
    diamond.nodes.emplace_back(1.0f, -1.0f, 2); // Bottom
    diamond.nodes.emplace_back(2.0f, 0.0f, 3); // End
    
    diamond.nodes[0].neighbors = {&diamond.nodes[1], &diamond.nodes[2]};
    diamond.nodes[1].neighbors = {&diamond.nodes[0], &diamond.nodes[3]};
    diamond.nodes[2].neighbors = {&diamond.nodes[0], &diamond.nodes[3]};
    diamond.nodes[3].neighbors = {&diamond.nodes[1], &diamond.nodes[2]};
    
    path = a_star_search(diamond, 0, 3);
    assert(path.size() == 3 && path[0].index == 0 && path[2].index == 3);
    
    // Test 3: Complex maze with multiple solutions
    Graph maze;
    for(int i = 0; i < 25; i++) {
        maze.nodes.emplace_back(i % 5 * 1.0f, i / 5 * 1.0f, i);
    }
    
    // Create maze connections (avoiding some to create walls)
    int connections[][2] = {{0,1},{1,2},{2,7},{7,12},{12,17},{17,22},{22,23},{23,24},
                           {0,5},{5,10},{10,15},{15,20},{20,21},{21,16},{16,11},{11,6},
                           {6,7},{8,9},{9,14},{14,19},{19,18},{18,13},{13,8}};
    
    for(auto& conn : connections) {
        maze.nodes[conn[0]].neighbors.push_back(&maze.nodes[conn[1]]);
        maze.nodes[conn[1]].neighbors.push_back(&maze.nodes[conn[0]]);
    }
    
    path = a_star_search(maze, 0, 24);
    assert(!path.empty() && path[0].index == 0 && path.back().index == 24);
    
    // Test 4: Densely connected graph
    Graph dense;
    for(int i = 0; i < 10; i++) {
        dense.nodes.emplace_back(i * 1.0f, 0.0f, i);
    }
    
    // Connect each node to all others
    for(int i = 0; i < 10; i++) {
        for(int j = 0; j < 10; j++) {
            if(i != j) dense.nodes[i].neighbors.push_back(&dense.nodes[j]);
        }
    }
    
    path = a_star_search(dense, 0, 9);
    assert(path.size() == 2); // Direct path should be optimal
    
    // Test 5: Tree structure with multiple branches
    Graph tree;
    for(int i = 0; i < 15; i++) {
        tree.nodes.emplace_back(i * 0.5f, (i % 3) * 1.0f, i);
    }
    
    // Binary tree connections
    for(int i = 0; i < 7; i++) {
        if(2*i+1 < 15) {
            tree.nodes[i].neighbors.push_back(&tree.nodes[2*i+1]);
            tree.nodes[2*i+1].neighbors.push_back(&tree.nodes[i]);
        }
        if(2*i+2 < 15) {
            tree.nodes[i].neighbors.push_back(&tree.nodes[2*i+2]);
            tree.nodes[2*i+2].neighbors.push_back(&tree.nodes[i]);
        }
    }
    
    path = a_star_search(tree, 0, 14);
    assert(!path.empty());
    
    std::cout << "test_complex_topologies passed!" << std::endl;
    return 0;
}