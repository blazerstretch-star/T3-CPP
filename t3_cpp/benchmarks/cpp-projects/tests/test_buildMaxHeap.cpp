#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    int arr[] = {1, 3, 2, 5, 4};
    Algorithms::buildMaxHeap(arr, 5);
    assert(arr[0] == 5);
    return 0;
}
