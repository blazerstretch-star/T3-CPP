#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    int arr[] = {5, 2, 8, 1, 9};
    int pivot = Algorithms::quickSortPartition(arr, 0, 4);
    assert(pivot >= 0 && pivot <= 4);
    return 0;
}
