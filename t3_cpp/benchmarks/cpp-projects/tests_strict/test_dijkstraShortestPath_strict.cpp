#include "../src/ds_algorithms.h"
#include <cassert>
#include <climits>

int main() {
    // Property: dist[start] = 0
    // Property: dist[v] <= dist[u] + edge(u,v) (triangle inequality)
    // Property: dist[unreachable] = INT_MAX
    
    auto verify_triangle_inequality = [](int graph[][100], int n, int dist[]) {
        for (int u = 0; u < n; u++) {
            for (int v = 0; v < n; v++) {
                if (graph[u][v] > 0 && dist[u] != INT_MAX) {
                    if (dist[v] > dist[u] + graph[u][v]) return false;
                }
            }
        }
        return true;
    };
    
    // Test 1: Single node
    {
        int graph[100][100] = {{0}};
        int dist[100];
        Algorithms::dijkstraShortestPath(graph, 1, 0, dist);
        assert(dist[0] == 0 && "Distance to self is 0");
    }
    
    // Test 2: Two connected nodes
    {
        int graph[100][100] = {{0, 5}, {0, 0}};
        int dist[100];
        Algorithms::dijkstraShortestPath(graph, 2, 0, dist);
        assert(dist[0] == 0 && "Distance to self is 0");
        assert(dist[1] == 5 && "Distance to neighbor");
    }
    
    // Test 3: Linear path
    {
        int graph[100][100] = {{0, 1, 0}, {0, 0, 2}, {0, 0, 0}};
        int dist[100];
        Algorithms::dijkstraShortestPath(graph, 3, 0, dist);
        assert(dist[0] == 0 && "Distance to self");
        assert(dist[1] == 1 && "Distance to node 1");
        assert(dist[2] == 3 && "Distance to node 2");
        assert(verify_triangle_inequality(graph, 3, dist) && "Triangle inequality");
    }
    
    // Test 4: Disconnected node
    {
        int graph[100][100] = {{0, 1, 0}, {0, 0, 0}, {0, 0, 0}};
        int dist[100];
        Algorithms::dijkstraShortestPath(graph, 3, 0, dist);
        assert(dist[0] == 0 && "Distance to self");
        assert(dist[1] == 1 && "Distance to connected");
        assert(dist[2] == INT_MAX && "Unreachable node");
    }
    
    // Test 5: Multiple paths
    {
        int graph[100][100] = {{0, 4, 2}, {0, 0, 1}, {0, 1, 0}};
        int dist[100];
        Algorithms::dijkstraShortestPath(graph, 3, 0, dist);
        assert(dist[0] == 0 && "Distance to self");
        assert(verify_triangle_inequality(graph, 3, dist) && "Triangle inequality");
    }
    
    // Test 6: Complete graph
    {
        int graph[100][100] = {{0, 1, 2}, {1, 0, 1}, {2, 1, 0}};
        int dist[100];
        Algorithms::dijkstraShortestPath(graph, 3, 0, dist);
        assert(dist[0] == 0 && "Distance to self");
        assert(verify_triangle_inequality(graph, 3, dist) && "Triangle inequality");
    }
    
    // Test 7: Larger graph
    {
        int graph[100][100] = {0};
        for (int i = 0; i < 5; i++) {
            if (i < 4) graph[i][i+1] = 1;
        }
        int dist[100];
        Algorithms::dijkstraShortestPath(graph, 5, 0, dist);
        for (int i = 0; i < 5; i++) {
            assert(dist[i] == i && "Distance equals hops");
        }
    }
    
    return 0;
}
