#include "../src/route_planning_functions.h"
#include <cassert>
#include <iostream>
#include <limits>

using namespace RoutePlanning;

int main() {
    // Test 1: Null pointer handling
    assert(calculate_h_value(nullptr, nullptr) == 0.0f);
    
    Node valid(0.0f, 0.0f);
    assert(calculate_h_value(&valid, nullptr) >= 0.0f);
    assert(calculate_h_value(nullptr, &valid) >= 0.0f);
    
    std::vector<Node> null_path = construct_path(nullptr);
    assert(null_path.empty());
    
    // Test 2: Invalid graph indices
    Graph invalid_graph;
    invalid_graph.nodes.emplace_back(0.0f, 0.0f, 0);
    
    assert(a_star_search(invalid_graph, -1, 0).empty());
    assert(a_star_search(invalid_graph, 0, -1).empty());
    assert(a_star_search(invalid_graph, 1000, 0).empty());
    assert(a_star_search(invalid_graph, 0, 1000).empty());
    
    // Test 3: Infinite and NaN coordinates
    Node inf_node(std::numeric_limits<float>::infinity(), 0.0f);
    Node nan_node(std::numeric_limits<float>::quiet_NaN(), 0.0f);
    Node normal_node(1.0f, 1.0f);
    
    float inf_dist = euclidean_distance(inf_node, normal_node);
    assert(std::isinf(inf_dist) || inf_dist > 1000000.0f);
    
    float nan_dist = euclidean_distance(nan_node, normal_node);
    assert(std::isnan(nan_dist) || nan_dist != nan_dist);
    
    // Test 4: Empty open list
    std::vector<Node*> empty_list;
    Node* result = find_next_node(empty_list);
    assert(result == nullptr);
    
    // Test 5: Circular parent references
    Node circular1(0.0f, 0.0f), circular2(1.0f, 1.0f);
    circular1.parent = &circular2;
    circular2.parent = &circular1;
    
    std::vector<Node> circular_path = construct_path(&circular1);
    assert(circular_path.size() <= 100); // Should detect cycle and limit
    
    // Test 6: Malformed graph structure
    Graph malformed;
    malformed.nodes.emplace_back(0.0f, 0.0f, 0);
    malformed.nodes.emplace_back(1.0f, 1.0f, 1);
    
    // Invalid neighbor pointer
    Node* invalid_neighbor = reinterpret_cast<Node*>(0xDEADBEEF);
    malformed.nodes[0].neighbors.push_back(invalid_neighbor);
    
    // Should handle gracefully without crashing
    std::vector<Node*> open_list;
    Node end(2.0f, 2.0f);
    add_neighbors_to_open_list(&malformed.nodes[0], open_list, &end, malformed);
    
    // Test 7: Extreme coordinate values
    Node extreme1(-1e10f, -1e10f);
    Node extreme2(1e10f, 1e10f);
    
    float extreme_dist = euclidean_distance(extreme1, extreme2);
    assert(extreme_dist > 0.0f);
    
    // Test 8: Invalid way indices in roads
    Graph way_invalid;
    way_invalid.nodes.emplace_back(0.0f, 0.0f, 0);
    
    Road bad_road;
    bad_road.type = Road::Type::Primary;
    bad_road.way = 999; // Non-existent way
    way_invalid.roads.push_back(bad_road);
    
    Node* way_result = find_closest_node(way_invalid, 0.0f, 0.0f);
    assert(way_result != nullptr); // Should fallback gracefully
    
    // Test 9: Empty ways with valid roads
    Graph empty_ways;
    empty_ways.nodes.emplace_back(5.0f, 5.0f, 0);
    
    Road empty_road;
    empty_road.type = Road::Type::Primary;
    empty_road.way = 0;
    empty_ways.roads.push_back(empty_road);
    
    Way empty_way;
    // empty_way.nodes is empty
    empty_ways.ways.push_back(empty_way);
    
    Node* empty_result = find_closest_node(empty_ways, 5.0f, 5.0f);
    assert(empty_result != nullptr);
    
    // Test 10: Negative g/h values
    Node neg_values(0.0f, 0.0f);
    neg_values.g_value = -1.0f;
    neg_values.h_value = -1.0f;
    
    std::vector<Node*> neg_list = {&neg_values};
    Node* neg_result = find_next_node(neg_list);
    assert(neg_result == &neg_values); // Should handle negative values
    
    std::cout << "test_malformed_input passed!" << std::endl;
    return 0;
}