#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    int arr[] = {1, 3, 5, 2, 4, 6};
    Algorithms::mergeArrays(arr, 0, 2, 5);
    assert(arr[0] == 1 && arr[5] == 6);
    return 0;
}
