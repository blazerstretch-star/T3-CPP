#include "../src/hospital_functions.h"
#include <cassert>
#include <iostream>
#include <limits>
#include <set>

int main() {
    // ========================================
    // PROPERTY 1: Path validity (all edges exist)
    // ========================================
    auto check_path_validity = [](const std::unordered_map<std::string, std::unordered_map<std::string, int>>& graph,
                                   const std::vector<std::string>& path) {
        assert(!path.empty() && "Path must not be empty");
        
        for (size_t i = 0; i + 1 < path.size(); ++i) {
            assert(graph.count(path[i]) > 0 && "Node must exist in graph");
            assert(graph.at(path[i]).count(path[i + 1]) > 0 && 
                   "Edge must exist between consecutive nodes in path");
        }
    };
    
    // ========================================
    // PROPERTY 2: Path endpoints correct
    // ========================================
    auto check_path_endpoints = [](const std::vector<std::string>& path,
                                    const std::string& source,
                                    const std::string& target) {
        assert(!path.empty() && "Path must not be empty");
        assert(path.front() == source && "Path must start at source");
        assert(path.back() == target && "Path must end at target");
    };
    
    // ========================================
    // PROPERTY 3: Distance matches path
    // ========================================
    auto check_distance_matches_path = [](const std::unordered_map<std::string, std::unordered_map<std::string, int>>& graph,
                                          const std::vector<std::string>& path,
                                          int distance) {
        int calculatedDistance = 0;
        for (size_t i = 0; i + 1 < path.size(); ++i) {
            calculatedDistance += graph.at(path[i]).at(path[i + 1]);
        }
        assert(calculatedDistance == distance && 
               "Reported distance must match sum of edge weights in path");
    };
    
    // ========================================
    // PROPERTY 4: Optimality (no shorter path exists via simple checks)
    // ========================================
    auto check_no_direct_shortcut = [](const std::unordered_map<std::string, std::unordered_map<std::string, int>>& graph,
                                       const std::vector<std::string>& path,
                                       int distance) {
        // Check if direct edge exists and is longer
        if (path.size() > 2) {
            const std::string& start = path.front();
            const std::string& end = path.back();
            if (graph.count(start) && graph.at(start).count(end)) {
                int directDist = graph.at(start).at(end);
                assert(distance <= directDist && 
                       "Shortest path must be <= direct edge");
            }
        }
    };
    
    // ========================================
    // PROPERTY 5: Path has no cycles
    // ========================================
    auto check_no_cycles = [](const std::vector<std::string>& path) {
        std::set<std::string> visited;
        for (const auto& node : path) {
            assert(visited.find(node) == visited.end() && 
                   "Path must not contain cycles");
            visited.insert(node);
        }
    };
    
    // ========================================
    // PROPERTY 6: Non-negative distance
    // ========================================
    auto check_non_negative_distance = [](int distance) {
        assert(distance >= 0 && "Distance must be non-negative");
    };
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: Simple linear path
    std::unordered_map<std::string, std::unordered_map<std::string, int>> graph1;
    graph1["A"] = {{"B", 5}};
    graph1["B"] = {{"C", 3}};
    graph1["C"] = {};
    
    auto result1 = dijkstraShortestPath(graph1, "A", "C");
    check_path_endpoints(result1.first, "A", "C");
    check_path_validity(graph1, result1.first);
    check_distance_matches_path(graph1, result1.first, result1.second);
    check_no_cycles(result1.first);
    check_non_negative_distance(result1.second);
    assert(result1.second == 8 && "A->B->C = 5+3 = 8");
    
    // Test 2: Multiple paths - choose shortest
    std::unordered_map<std::string, std::unordered_map<std::string, int>> graph2;
    graph2["A"] = {{"B", 5}, {"C", 2}};
    graph2["B"] = {{"D", 1}};
    graph2["C"] = {{"D", 6}};
    graph2["D"] = {};
    
    auto result2 = dijkstraShortestPath(graph2, "A", "D");
    check_path_endpoints(result2.first, "A", "D");
    check_path_validity(graph2, result2.first);
    check_distance_matches_path(graph2, result2.first, result2.second);
    check_no_cycles(result2.first);
    check_non_negative_distance(result2.second);
    check_no_direct_shortcut(graph2, result2.first, result2.second);
    assert(result2.second == 6 && "Shortest: A->B->D = 5+1 = 6");
    
    // Test 3: Direct edge vs longer path
    std::unordered_map<std::string, std::unordered_map<std::string, int>> graph3;
    graph3["A"] = {{"B", 10}, {"C", 3}};
    graph3["B"] = {};
    graph3["C"] = {{"B", 2}};
    
    auto result3 = dijkstraShortestPath(graph3, "A", "B");
    check_path_endpoints(result3.first, "A", "B");
    check_path_validity(graph3, result3.first);
    check_distance_matches_path(graph3, result3.first, result3.second);
    check_no_cycles(result3.first);
    check_non_negative_distance(result3.second);
    assert(result3.second == 5 && "Shortest: A->C->B = 3+2 = 5 (not direct 10)");
    
    // Test 4: Single node (source == target)
    std::unordered_map<std::string, std::unordered_map<std::string, int>> graph4;
    graph4["A"] = {};
    
    auto result4 = dijkstraShortestPath(graph4, "A", "A");
    check_path_endpoints(result4.first, "A", "A");
    assert(result4.second == 0 && "Distance to self is 0");
    assert(result4.first.size() == 1 && "Path to self has one node");
    
    // Test 5: Complex graph with multiple paths
    std::unordered_map<std::string, std::unordered_map<std::string, int>> graph5;
    graph5["A"] = {{"B", 4}, {"C", 2}};
    graph5["B"] = {{"C", 1}, {"D", 5}};
    graph5["C"] = {{"D", 8}, {"E", 10}};
    graph5["D"] = {{"E", 2}};
    graph5["E"] = {};
    
    auto result5 = dijkstraShortestPath(graph5, "A", "E");
    check_path_endpoints(result5.first, "A", "E");
    check_path_validity(graph5, result5.first);
    check_distance_matches_path(graph5, result5.first, result5.second);
    check_no_cycles(result5.first);
    check_non_negative_distance(result5.second);
    check_no_direct_shortcut(graph5, result5.first, result5.second);
    assert(result5.second == 11 && "Shortest: A->B->D->E = 4+5+2 = 11");
    
    // Test 6: Equal weight paths (any valid shortest path acceptable)
    std::unordered_map<std::string, std::unordered_map<std::string, int>> graph6;
    graph6["A"] = {{"B", 1}, {"C", 1}};
    graph6["B"] = {{"D", 1}};
    graph6["C"] = {{"D", 1}};
    graph6["D"] = {};
    
    auto result6 = dijkstraShortestPath(graph6, "A", "D");
    check_path_endpoints(result6.first, "A", "D");
    check_path_validity(graph6, result6.first);
    check_distance_matches_path(graph6, result6.first, result6.second);
    check_no_cycles(result6.first);
    assert(result6.second == 2 && "Both paths have distance 2");
    
    // Test 7: Large graph stress test
    std::unordered_map<std::string, std::unordered_map<std::string, int>> graph7;
    for (int i = 0; i < 50; i++) {
        std::string node = "N" + std::to_string(i);
        graph7[node] = {};
        if (i > 0) {
            graph7["N" + std::to_string(i - 1)][node] = 1;
        }
        if (i > 1) {
            graph7["N" + std::to_string(i - 2)][node] = 3;
        }
    }
    
    auto result7 = dijkstraShortestPath(graph7, "N0", "N49");
    check_path_endpoints(result7.first, "N0", "N49");
    check_path_validity(graph7, result7.first);
    check_distance_matches_path(graph7, result7.first, result7.second);
    check_no_cycles(result7.first);
    check_non_negative_distance(result7.second);
    assert(result7.second == 49 && "Linear path with unit weights");
    
    // Test 8: Triangle inequality
    std::unordered_map<std::string, std::unordered_map<std::string, int>> graph8;
    graph8["A"] = {{"B", 3}, {"C", 5}};
    graph8["B"] = {{"C", 1}};
    graph8["C"] = {};
    
    auto result8 = dijkstraShortestPath(graph8, "A", "C");
    check_path_endpoints(result8.first, "A", "C");
    check_path_validity(graph8, result8.first);
    check_distance_matches_path(graph8, result8.first, result8.second);
    assert(result8.second == 4 && "A->B->C = 3+1 = 4 < direct 5");
    
    std::cout << "All dijkstraShortestPath strict tests passed!" << std::endl;
    return 0;
}
