#include "../src/route_planning_functions.h"
#include <cassert>
#include <iostream>
#include <chrono>
#include <vector>
#include <memory>

using namespace RoutePlanning;

int main() {
    // Test 1: Memory allocation stress
    {
        std::vector<std::unique_ptr<Graph>> graphs;
        for(int i = 0; i < 100; i++) {
            auto graph = std::make_unique<Graph>();
            for(int j = 0; j < 50; j++) {
                graph->nodes.emplace_back(j * 0.1f, i * 0.1f, j);
            }
            graphs.push_back(std::move(graph));
        }
        // All graphs should be automatically cleaned up
    }
    
    // Test 2: Deep recursion path construction
    Graph deep;
    for(int i = 0; i < 1000; i++) {
        deep.nodes.emplace_back(i * 0.01f, 0.0f, i);
    }
    
    // Create linear chain
    for(int i = 0; i < 999; i++) {
        deep.nodes[i+1].parent = &deep.nodes[i];
    }
    
    std::vector<Node> deep_path = construct_path(&deep.nodes[999]);
    assert(deep_path.size() == 1000);
    
    // Test 3: Large open list performance
    std::vector<Node> nodes(10000);
    std::vector<Node*> large_open_list;
    
    for(int i = 0; i < 10000; i++) {
        nodes[i] = Node(i * 0.001f, 0.0f);
        nodes[i].g_value = i * 0.1f;
        nodes[i].h_value = (10000 - i) * 0.1f;
        large_open_list.push_back(&nodes[i]);
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < 100; i++) {
        std::vector<Node*> temp_list = large_open_list;
        Node* next = find_next_node(temp_list);
        assert(next != nullptr);
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    assert(duration.count() < 5000); // Should complete within 5 seconds
    
    // Test 4: Repeated distance calculations
    Node base(0.0f, 0.0f);
    std::vector<Node> distance_nodes(1000);
    
    for(int i = 0; i < 1000; i++) {
        distance_nodes[i] = Node(i * 0.1f, i * 0.1f);
    }
    
    start = std::chrono::high_resolution_clock::now();
    float total_distance = 0.0f;
    for(int iter = 0; iter < 1000; iter++) {
        for(int i = 0; i < 1000; i++) {
            total_distance += euclidean_distance(base, distance_nodes[i]);
        }
    }
    end = std::chrono::high_resolution_clock::now();
    
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    assert(duration.count() < 1000); // Should be fast
    assert(total_distance > 0.0f);
    
    // Test 5: Graph with many neighbors per node
    Graph dense_neighbors;
    for(int i = 0; i < 100; i++) {
        dense_neighbors.nodes.emplace_back(i * 0.1f, 0.0f, i);
    }
    
    // Each node connects to 50 others
    for(int i = 0; i < 100; i++) {
        for(int j = 0; j < 50; j++) {
            int neighbor_idx = (i + j + 1) % 100;
            dense_neighbors.nodes[i].neighbors.push_back(&dense_neighbors.nodes[neighbor_idx]);
        }
    }
    
    start = std::chrono::high_resolution_clock::now();
    std::vector<Node> dense_path = a_star_search(dense_neighbors, 0, 99);
    end = std::chrono::high_resolution_clock::now();
    
    assert(!dense_path.empty());
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    assert(duration.count() < 2000);
    
    // Test 6: Memory fragmentation test
    std::vector<Graph*> fragmented_graphs;
    for(int i = 0; i < 1000; i++) {
        Graph* g = new Graph();
        g->nodes.emplace_back(i * 0.01f, 0.0f, 0);
        fragmented_graphs.push_back(g);
        
        if(i % 2 == 0 && i > 0) {
            delete fragmented_graphs[i-1];
            fragmented_graphs[i-1] = nullptr;
        }
    }
    
    // Cleanup remaining graphs
    for(Graph* g : fragmented_graphs) {
        delete g;
    }
    
    std::cout << "test_performance_memory passed!" << std::endl;
    return 0;
}