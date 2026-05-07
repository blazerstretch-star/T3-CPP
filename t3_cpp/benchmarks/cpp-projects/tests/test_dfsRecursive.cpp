#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    int graph[100][100] = {{0, 1, 0}, {0, 0, 1}, {0, 0, 0}};
    bool visited[100] = {false};
    Algorithms::dfsRecursive(graph, 3, 0, visited);
    assert(visited[0] == true);
    return 0;
}
