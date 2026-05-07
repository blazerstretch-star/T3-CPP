#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    // Property: All reachable nodes are visited
    // Property: visited[start] = true
    
    // Test 1: Single node
    {
        int graph[100][100] = {{0}};
        bool visited[100] = {false};
        Algorithms::bfsIterative(graph, 1, 0, visited);
        assert(visited[0] && "Start node visited");
    }
    
    // Test 2: Two connected nodes
    {
        int graph[100][100] = {{0, 1}, {1, 0}};
        bool visited[100] = {false};
        Algorithms::bfsIterative(graph, 2, 0, visited);
        assert(visited[0] && visited[1] && "Both nodes visited");
    }
    
    // Test 3: Linear chain
    {
        int graph[100][100] = {{0, 1, 0}, {0, 0, 1}, {0, 0, 0}};
        bool visited[100] = {false};
        Algorithms::bfsIterative(graph, 3, 0, visited);
        assert(visited[0] && visited[1] && visited[2] && "All nodes visited");
    }
    
    // Test 4: Disconnected graph
    {
        int graph[100][100] = {{0, 1, 0}, {1, 0, 0}, {0, 0, 0}};
        bool visited[100] = {false};
        Algorithms::bfsIterative(graph, 3, 0, visited);
        assert(visited[0] && visited[1] && "Connected nodes visited");
        assert(!visited[2] && "Disconnected node not visited");
    }
    
    // Test 5: Cycle
    {
        int graph[100][100] = {{0, 1, 0}, {0, 0, 1}, {1, 0, 0}};
        bool visited[100] = {false};
        Algorithms::bfsIterative(graph, 3, 0, visited);
        assert(visited[0] && visited[1] && visited[2] && "All nodes in cycle visited");
    }
    
    // Test 6: Complete graph
    {
        int graph[100][100] = {{0, 1, 1}, {1, 0, 1}, {1, 1, 0}};
        bool visited[100] = {false};
        Algorithms::bfsIterative(graph, 3, 0, visited);
        assert(visited[0] && visited[1] && visited[2] && "All nodes visited");
    }
    
    // Test 7: Larger graph
    {
        int graph[100][100] = {0};
        for (int i = 0; i < 5; i++) {
            if (i < 4) graph[i][i+1] = 1;
        }
        bool visited[100] = {false};
        Algorithms::bfsIterative(graph, 5, 0, visited);
        for (int i = 0; i < 5; i++) {
            assert(visited[i] && "All nodes in path visited");
        }
    }
    
    return 0;
}
