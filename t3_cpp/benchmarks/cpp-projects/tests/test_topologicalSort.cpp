#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    int graph[100][100] = {{0, 1, 0}, {0, 0, 1}, {0, 0, 0}};
    int result[100];
    Algorithms::topologicalSort(graph, 3, result);
    assert(result[0] >= 0);
    return 0;
}
