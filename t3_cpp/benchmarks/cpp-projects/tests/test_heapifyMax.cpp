#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    int arr[] = {1, 3, 2};
    Algorithms::heapifyMax(arr, 3, 0);
    assert(arr[0] == 3);
    return 0;
}
