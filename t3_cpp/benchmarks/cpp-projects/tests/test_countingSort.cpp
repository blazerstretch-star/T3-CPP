#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    int arr[] = {5, 2, 8, 1, 9};
    Algorithms::countingSort(arr, 5, 9);
    assert(arr[0] == 1 && arr[4] == 9);
    return 0;
}
