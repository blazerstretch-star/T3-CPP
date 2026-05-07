#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    int arr[] = {1, 2, 3, 4, 5};
    Algorithms::reverseArray(arr, 5);
    assert(arr[0] == 5 && arr[4] == 1);
    return 0;
}
