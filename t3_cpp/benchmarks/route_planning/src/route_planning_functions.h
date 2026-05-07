#pragma once
#include <vector>
#include <limits>
#include <cmath>
#include <unordered_map>
#include <algorithm>

// Forward declarations
class Model;

namespace RoutePlanning {

// Node structure for pathfinding
struct Node {
    float x, y;
    Node* parent = nullptr;
    float h_value = std::numeric_limits<float>::max();
    float g_value = 0.0;
    bool visited = false;
    std::vector<Node*> neighbors;
    int index = -1;
    
    Node() = default;
    Node(float x_val, float y_val, int idx = -1) : x(x_val), y(y_val), index(idx) {}
    
    float distance(const Node& other) const {
        return std::sqrt(std::pow((x - other.x), 2) + std::pow((y - other.y), 2));
    }
};

// Road structure
struct Road {
    enum Type { Motorway, Trunk, Primary, Secondary, Tertiary, Unclassified, Residential, Footway };
    Type type;
    int way;
};

// Way structure  
struct Way {
    std::vector<int> nodes;
};

// Graph structure
struct Graph {
    std::vector<Node> nodes;
    std::vector<Road> roads;
    std::vector<Way> ways;
    std::unordered_map<int, std::vector<const Road*>> node_to_road;
    float metric_scale = 1.0f;
};

// Function declarations
float calculate_h_value(const Node* node, const Node* end_node);
Node* find_next_node(std::vector<Node*>& open_list);
void add_neighbors_to_open_list(Node* current_node, std::vector<Node*>& open_list, const Node* end_node, const Graph& graph);
std::vector<Node> construct_path(Node* end_node);
std::vector<Node> a_star_search(Graph& graph, int start_idx, int end_idx);
Node* find_closest_node(const Graph& graph, float x, float y);
float euclidean_distance(const Node& a, const Node& b);

}