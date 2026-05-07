#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    int arr[10];
    int size = 0;
    assert(Algorithms::priorityQueueEnqueue(arr, size, 10, 5) == true);
    assert(size == 1 && arr[0] == 5);
    return 0;
}
