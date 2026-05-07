#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    // Property: For every edge u->v, u appears before v in result
    // Property: All nodes appear exactly once
    
    auto verify_topological_order = [](int graph[][100], int n, int result[]) {
        // Create position map
        int pos[100];
        for (int i = 0; i < n; i++) {
            pos[result[i]] = i;
        }
        // Check all edges
        for (int u = 0; u < n; u++) {
            for (int v = 0; v < n; v++) {
                if (graph[u][v]) {
                    if (pos[u] >= pos[v]) return false;
                }
            }
        }
        return true;
    };
    
    // Test 1: Single node
    {
        int graph[100][100] = {{0}};
        int result[100];
        Algorithms::topologicalSort(graph, 1, result);
        assert(result[0] == 0 && "Single node");
    }
    
    // Test 2: Two nodes with edge
    {
        int graph[100][100] = {{0, 1}, {0, 0}};
        int result[100];
        Algorithms::topologicalSort(graph, 2, result);
        assert(verify_topological_order(graph, 2, result) && "Topological order");
    }
    
    // Test 3: Linear chain
    {
        int graph[100][100] = {{0, 1, 0}, {0, 0, 1}, {0, 0, 0}};
        int result[100];
        Algorithms::topologicalSort(graph, 3, result);
        assert(verify_topological_order(graph, 3, result) && "Topological order");
    }
    
    // Test 4: Diamond shape
    {
        int graph[100][100] = {{0, 1, 1, 0}, {0, 0, 0, 1}, {0, 0, 0, 1}, {0, 0, 0, 0}};
        int result[100];
        Algorithms::topologicalSort(graph, 4, result);
        assert(verify_topological_order(graph, 4, result) && "Topological order");
    }
    
    // Test 5: Multiple sources
    {
        int graph[100][100] = {{0, 0, 1}, {0, 0, 1}, {0, 0, 0}};
        int result[100];
        Algorithms::topologicalSort(graph, 3, result);
        assert(verify_topological_order(graph, 3, result) && "Topological order");
    }
    
    // Test 6: No edges
    {
        int graph[100][100] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
        int result[100];
        Algorithms::topologicalSort(graph, 3, result);
        // Any order is valid
    }
    
    // Test 7: Larger DAG
    {
        int graph[100][100] = {0};
        graph[0][1] = 1;
        graph[0][2] = 1;
        graph[1][3] = 1;
        graph[2][3] = 1;
        graph[3][4] = 1;
        int result[100];
        Algorithms::topologicalSort(graph, 5, result);
        assert(verify_topological_order(graph, 5, result) && "Topological order");
    }
    
    return 0;
}
