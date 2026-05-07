#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    int arr[] = {3, 1, 2};
    Algorithms::heapifyMin(arr, 3, 0);
    assert(arr[0] == 1);
    return 0;
}
