#include "../src/route_planning_functions.h"
#include <cassert>
#include <iostream>

using namespace RoutePlanning;

int main() {
    // Test 1: Basic path finding
    Graph graph;
    graph.nodes.emplace_back(0.0f, 0.0f, 0);
    graph.nodes.emplace_back(1.0f, 0.0f, 1);
    graph.nodes.emplace_back(2.0f, 0.0f, 2);
    
    graph.nodes[0].neighbors = {&graph.nodes[1]};
    graph.nodes[1].neighbors = {&graph.nodes[0], &graph.nodes[2]};
    graph.nodes[2].neighbors = {&graph.nodes[1]};
    
    std::vector<Node> path = a_star_search(graph, 0, 2);
    assert(!path.empty() && path.size() == 3);
    assert(path[0].index == 0 && path[2].index == 2);
    
    // Test 2: Invalid indices
    assert(a_star_search(graph, -1, 0).empty());
    assert(a_star_search(graph, 0, 10).empty());
    assert(a_star_search(graph, 10, 0).empty());
    
    // Test 3: Same start and end
    path = a_star_search(graph, 1, 1);
    assert(path.size() == 1 && path[0].index == 1);
    
    // Test 4: Disconnected graph
    Graph disconnected;
    disconnected.nodes.emplace_back(0.0f, 0.0f, 0);
    disconnected.nodes.emplace_back(5.0f, 5.0f, 1);
    assert(a_star_search(disconnected, 0, 1).empty());
    
    // Test 5: Complex graph with optimal path
    Graph complex;
    for(int i = 0; i < 5; i++) {
        complex.nodes.emplace_back(i * 1.0f, 0.0f, i);
    }
    complex.nodes[0].neighbors = {&complex.nodes[1], &complex.nodes[3]};
    complex.nodes[1].neighbors = {&complex.nodes[0], &complex.nodes[2]};
    complex.nodes[2].neighbors = {&complex.nodes[1], &complex.nodes[4]};
    complex.nodes[3].neighbors = {&complex.nodes[0], &complex.nodes[4]};
    complex.nodes[4].neighbors = {&complex.nodes[2], &complex.nodes[3]};
    
    path = a_star_search(complex, 0, 4);
    assert(!path.empty());
    assert(path[0].index == 0 && path.back().index == 4);
    
    std::cout << "test_a_star_search passed!" << std::endl;
    return 0;
}