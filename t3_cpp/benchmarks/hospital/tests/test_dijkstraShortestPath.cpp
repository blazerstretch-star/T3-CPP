#include "../src/hospital_functions.h"
#include <cassert>
#include <iostream>
#include <unordered_set>

void test_dijkstraShortestPath() {
    // Test basic shortest path
    std::unordered_map<std::string, std::unordered_map<std::string, int>> graph;
    graph["A"] = {{"B", 5}, {"C", 2}};
    graph["B"] = {{"D", 1}};
    graph["C"] = {{"D", 6}};
    graph["D"] = {};
    
    auto result = dijkstraShortestPath(graph, "A", "D");
    assert(result.second == 6);
    assert(result.first.size() == 3);
    assert(result.first[0] == "A");
    assert(result.first[1] == "B");
    assert(result.first[2] == "D");
    
    // Test complex graph with multiple paths
    std::unordered_map<std::string, std::unordered_map<std::string, int>> complexGraph;
    complexGraph["A"] = {{"B", 4}, {"C", 2}};
    complexGraph["B"] = {{"C", 1}, {"D", 5}};
    complexGraph["C"] = {{"D", 8}, {"E", 10}};
    complexGraph["D"] = {{"E", 2}};
    complexGraph["E"] = {};
    
    auto complexResult = dijkstraShortestPath(complexGraph, "A", "E");
    assert(complexResult.second == 11); // A->B->D->E = 4+5+2 = 11
    
    std::cout << "test_dijkstraShortestPath passed!" << std::endl;
}

int main() {
    test_dijkstraShortestPath();
    return 0;
}