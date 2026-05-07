#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    int arr[] = {5, 3, 2, 1, 4};
    Algorithms::buildMinHeap(arr, 5);
    assert(arr[0] == 1);
    return 0;
}
