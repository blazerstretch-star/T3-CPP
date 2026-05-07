#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    int arr[] = {5, 2, 8, 1, 9};
    Algorithms::mergeSort(arr, 0, 4);
    assert(arr[0] == 1 && arr[4] == 9);
    return 0;
}
