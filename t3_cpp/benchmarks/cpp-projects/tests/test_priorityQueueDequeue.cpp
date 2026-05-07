#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    int arr[] = {5, 3, 2};
    int size = 3;
    int value;
    assert(Algorithms::priorityQueueDequeue(arr, size, value) == true);
    assert(value == 5 && size == 2);
    return 0;
}
