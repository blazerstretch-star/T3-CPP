#include "../src/route_planning_functions.h"
#include <cassert>
#include <iostream>

using namespace RoutePlanning;

int main() {
    // Test 1: Basic closest node finding
    Graph graph;
    graph.nodes.emplace_back(0.0f, 0.0f, 0);
    graph.nodes.emplace_back(5.0f, 0.0f, 1);
    graph.nodes.emplace_back(10.0f, 0.0f, 2);
    
    Road road;
    road.type = Road::Type::Primary;
    road.way = 0;
    graph.roads.push_back(road);
    
    Way way;
    way.nodes = {0, 1, 2};
    graph.ways.push_back(way);
    
    Node* closest = find_closest_node(graph, 1.0f, 0.0f);
    assert(closest != nullptr && closest->x == 0.0f);
    
    // Test 2: Multiple road types
    Road footway;
    footway.type = Road::Type::Footway;
    footway.way = 1;
    graph.roads.push_back(footway);
    
    Way footway_way;
    footway_way.nodes = {1};
    graph.ways.push_back(footway_way);
    
    closest = find_closest_node(graph, 5.5f, 0.0f);
    assert(closest->x == 5.0f); // Should ignore footway
    
    // Test 3: Different road types
    Road secondary;
    secondary.type = Road::Type::Secondary;
    secondary.way = 2;
    graph.roads.push_back(secondary);
    
    Way secondary_way;
    secondary_way.nodes = {2};
    graph.ways.push_back(secondary_way);
    
    closest = find_closest_node(graph, 9.0f, 0.0f);
    assert(closest->x == 10.0f);
    
    // Test 4: Empty roads
    Graph empty_graph;
    empty_graph.nodes.emplace_back(0.0f, 0.0f, 0);
    Node* empty_result = find_closest_node(empty_graph, 0.0f, 0.0f);
    assert(empty_result->x == 0.0f);
    
    // Test 5: Out of bounds node indices
    Road invalid_road;
    invalid_road.type = Road::Type::Primary;
    invalid_road.way = 0;
    
    Graph bounds_graph;
    bounds_graph.nodes.emplace_back(1.0f, 1.0f, 0);
    bounds_graph.roads.push_back(invalid_road);
    
    Way invalid_way;
    invalid_way.nodes = {0, 10}; // Index 10 doesn't exist
    bounds_graph.ways.push_back(invalid_way);
    
    Node* bounds_result = find_closest_node(bounds_graph, 1.1f, 1.1f);
    assert(bounds_result->x == 1.0f);
    
    // Test 6: Negative coordinates
    Graph neg_graph;
    neg_graph.nodes.emplace_back(-5.0f, -5.0f, 0);
    neg_graph.nodes.emplace_back(5.0f, 5.0f, 1);
    
    Road neg_road;
    neg_road.type = Road::Type::Primary;
    neg_road.way = 0;
    neg_graph.roads.push_back(neg_road);
    
    Way neg_way;
    neg_way.nodes = {0, 1};
    neg_graph.ways.push_back(neg_way);
    
    Node* neg_result = find_closest_node(neg_graph, -4.0f, -4.0f);
    assert(neg_result->x == -5.0f);
    
    std::cout << "test_find_closest_node passed!" << std::endl;
    return 0;
}