#include "../src/route_planning_functions.h"
#include <cassert>
#include <iostream>

using namespace RoutePlanning;

int main() {
    // Test 1: Basic neighbor addition
    Node current(0.0f, 0.0f);
    Node neighbor1(1.0f, 0.0f);
    Node neighbor2(0.0f, 1.0f);
    Node end_node(2.0f, 2.0f);
    
    current.neighbors = {&neighbor1, &neighbor2};
    current.g_value = 1.0f;
    
    std::vector<Node*> open_list;
    Graph graph;
    
    add_neighbors_to_open_list(&current, open_list, &end_node, graph);
    
    assert(open_list.size() == 2);
    assert(neighbor1.visited && neighbor2.visited);
    assert(neighbor1.parent == &current && neighbor2.parent == &current);
    assert(neighbor1.g_value > 0.0f && neighbor2.g_value > 0.0f);
    
    // Test 2: Already visited neighbors
    Node current2(3.0f, 3.0f);
    Node visited_neighbor(4.0f, 3.0f);
    visited_neighbor.visited = true;
    current2.neighbors = {&visited_neighbor};
    
    std::vector<Node*> open_list2;
    add_neighbors_to_open_list(&current2, open_list2, &end_node, graph);
    assert(open_list2.empty());
    
    // Test 3: No neighbors
    Node isolated(5.0f, 5.0f);
    std::vector<Node*> open_list3;
    add_neighbors_to_open_list(&isolated, open_list3, &end_node, graph);
    assert(open_list3.empty());
    
    // Test 4: Multiple calls (duplicate handling)
    Node current3(6.0f, 6.0f);
    Node neighbor3(7.0f, 6.0f);
    current3.neighbors = {&neighbor3};
    
    std::vector<Node*> open_list4;
    add_neighbors_to_open_list(&current3, open_list4, &end_node, graph);
    assert(open_list4.size() == 1);
    
    // Second call should not add again (already visited)
    add_neighbors_to_open_list(&current3, open_list4, &end_node, graph);
    assert(open_list4.size() == 1);
    
    // Test 5: G-value calculation accuracy
    Node precise_current(0.0f, 0.0f);
    Node precise_neighbor(3.0f, 4.0f); // Distance = 5
    precise_current.neighbors = {&precise_neighbor};
    precise_current.g_value = 2.0f;
    
    std::vector<Node*> open_list5;
    add_neighbors_to_open_list(&precise_current, open_list5, &end_node, graph);
    assert(std::abs(precise_neighbor.g_value - 7.0f) < 0.001f);
    
    std::cout << "test_add_neighbors_to_open_list passed!" << std::endl;
    return 0;
}