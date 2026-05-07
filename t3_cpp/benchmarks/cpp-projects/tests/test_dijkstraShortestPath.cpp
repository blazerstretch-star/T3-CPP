#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    int graph[100][100] = {{0, 4, 0}, {0, 0, 8}, {0, 0, 0}};
    int dist[100];
    Algorithms::dijkstraShortestPath(graph, 3, 0, dist);
    assert(dist[0] == 0);
    return 0;
}
