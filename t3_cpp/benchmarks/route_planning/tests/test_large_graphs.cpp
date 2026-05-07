#include "../src/route_planning_functions.h"
#include <cassert>
#include <iostream>
#include <chrono>
#include <random>

using namespace RoutePlanning;

int main() {
    // Test 1: 100-node grid graph
    Graph grid;
    for(int i = 0; i < 100; i++) {
        grid.nodes.emplace_back(i % 10 * 1.0f, i / 10 * 1.0f, i);
    }
    
    // Connect grid neighbors
    for(int i = 0; i < 100; i++) {
        int x = i % 10, y = i / 10;
        if(x > 0) grid.nodes[i].neighbors.push_back(&grid.nodes[i-1]);
        if(x < 9) grid.nodes[i].neighbors.push_back(&grid.nodes[i+1]);
        if(y > 0) grid.nodes[i].neighbors.push_back(&grid.nodes[i-10]);
        if(y < 9) grid.nodes[i].neighbors.push_back(&grid.nodes[i+10]);
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<Node> path = a_star_search(grid, 0, 99);
    auto end = std::chrono::high_resolution_clock::now();
    
    assert(!path.empty());
    assert(path[0].index == 0 && path.back().index == 99);
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    assert(duration.count() < 1000); // Should complete within 1 second
    
    // Test 2: 500-node random graph
    Graph random;
    std::mt19937 gen(42);
    std::uniform_real_distribution<float> pos_dist(0.0f, 50.0f);
    
    for(int i = 0; i < 500; i++) {
        random.nodes.emplace_back(pos_dist(gen), pos_dist(gen), i);
    }
    
    // Connect each node to 5 random neighbors
    std::uniform_int_distribution<int> node_dist(0, 499);
    for(int i = 0; i < 500; i++) {
        for(int j = 0; j < 5; j++) {
            int neighbor = node_dist(gen);
            if(neighbor != i) {
                random.nodes[i].neighbors.push_back(&random.nodes[neighbor]);
            }
        }
    }
    
    start = std::chrono::high_resolution_clock::now();
    path = a_star_search(random, 0, 499);
    end = std::chrono::high_resolution_clock::now();
    
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    assert(duration.count() < 5000); // Should complete within 5 seconds
    
    // Test 3: Memory stress test - 1000 nodes
    Graph stress;
    for(int i = 0; i < 1000; i++) {
        stress.nodes.emplace_back(i * 0.1f, (i % 100) * 0.1f, i);
    }
    
    // Linear chain
    for(int i = 0; i < 999; i++) {
        stress.nodes[i].neighbors.push_back(&stress.nodes[i+1]);
        stress.nodes[i+1].neighbors.push_back(&stress.nodes[i]);
    }
    
    start = std::chrono::high_resolution_clock::now();
    path = a_star_search(stress, 0, 999);
    end = std::chrono::high_resolution_clock::now();
    
    assert(path.size() == 1000);
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    assert(duration.count() < 10000); // Should complete within 10 seconds
    
    std::cout << "test_large_graphs passed!" << std::endl;
    return 0;
}