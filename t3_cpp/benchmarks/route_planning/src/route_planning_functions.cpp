#include "route_planning_functions.h"
// Standard C++ Library (pre-included for agent evaluation)
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <queue>
#include <stack>
#include <deque>
#include <list>
#include <algorithm>
#include <cmath>
#include <string>
#include <memory>
#include <utility>


namespace RoutePlanning {

float calculate_h_value(const Node* node, const Node* end_node) {
    // FUNCTION_ID: route_planning_func001 - START
    if (!node || !end_node) return 0.0f;
    return node->distance(*end_node);
    // FUNCTION_ID: route_planning_func001 - END
}

Node* find_next_node(std::vector<Node*>& open_list) {
    // FUNCTION_ID: route_planning_func002 - START
    if (open_list.empty()) return nullptr;
    std::sort(open_list.begin(), open_list.end(), [](const Node* a, const Node* b) {
        return (a->h_value + a->g_value) < (b->h_value + b->g_value);
    });
    Node* lowest_node = open_list.front();
    open_list.erase(open_list.begin());
    return lowest_node;
    // FUNCTION_ID: route_planning_func002 - END
}

void add_neighbors_to_open_list(Node* current_node, std::vector<Node*>& open_list, const Node* end_node, const Graph& graph) {
    // FUNCTION_ID: route_planning_func003 - START
    for (Node* neighbor : current_node->neighbors) {
        if (neighbor && !neighbor->visited) {
            neighbor->parent = current_node;
            neighbor->g_value = current_node->g_value + current_node->distance(*neighbor);
            neighbor->h_value = calculate_h_value(neighbor, end_node);
            open_list.push_back(neighbor);
            neighbor->visited = true;
        }
    }
    // FUNCTION_ID: route_planning_func003 - END
}

std::vector<Node> construct_path(Node* end_node) {
    // FUNCTION_ID: route_planning_func004 - START
    std::vector<Node> path;
    if (!end_node) return path;
    Node* current = end_node;
    std::set<Node*> visited;
    while (current->parent != nullptr) {
        path.push_back(*current);
        visited.insert(current);
        current = current->parent;
        if (visited.count(current) > 0) break; // Cycle detected
    }
    path.push_back(*current);
    std::reverse(path.begin(), path.end());
    return path;
    // FUNCTION_ID: route_planning_func004 - END
}

std::vector<Node> a_star_search(Graph& graph, int start_idx, int end_idx) {
    // FUNCTION_ID: route_planning_func005 - START
    if (start_idx < 0 || start_idx >= graph.nodes.size() || end_idx < 0 || end_idx >= graph.nodes.size()) {
        return {};
    }
    
    Node* start_node = &graph.nodes[start_idx];
    Node* end_node = &graph.nodes[end_idx];
    
    // Handle case where start == end
    if (start_idx == end_idx) {
        return {*start_node};
    }
    
    std::vector<Node*> open_list;
    start_node->visited = true;
    open_list.push_back(start_node);
    
    while (!open_list.empty()) {
        Node* current_node = find_next_node(open_list);
        
        if (current_node->distance(*end_node) == 0) {
            return construct_path(end_node);
        }
        
        add_neighbors_to_open_list(current_node, open_list, end_node, graph);
    }
    
    return {};
    // FUNCTION_ID: route_planning_func005 - END
}

Node* find_closest_node(const Graph& graph, float x, float y) {
    // FUNCTION_ID: route_planning_func006 - START
    if (graph.nodes.empty()) return nullptr;
    Node input(x, y);
    
    float min_dist = std::numeric_limits<float>::max();
    int closest_idx = 0;
    
    for (const auto& road : graph.roads) {
        if (road.way < graph.ways.size() && road.type != Road::Type::Footway) {
            for (int node_idx : graph.ways[road.way].nodes) {
                if (node_idx < graph.nodes.size()) {
                    float dist = input.distance(graph.nodes[node_idx]);
                    if (dist < min_dist) {
                        closest_idx = node_idx;
                        min_dist = dist;
                    }
                }
            }
        }
    }
    
    return const_cast<Node*>(&graph.nodes[closest_idx]);
    // FUNCTION_ID: route_planning_func006 - END
}

float euclidean_distance(const Node& a, const Node& b) {
    // FUNCTION_ID: route_planning_func007 - START
    return std::sqrt(std::pow((a.x - b.x), 2) + std::pow((a.y - b.y), 2));
    // FUNCTION_ID: route_planning_func007 - END
}

}
