#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    int arr[] = {1, 2, 5, 8, 9};
    assert(Algorithms::binarySearchRecursive(arr, 0, 4, 8) == 3);
    assert(Algorithms::binarySearchRecursive(arr, 0, 4, 10) == -1);
    return 0;
}
