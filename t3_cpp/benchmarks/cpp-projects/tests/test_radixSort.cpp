#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    int arr[] = {170, 45, 75, 90, 802, 24, 2, 66};
    Algorithms::radixSort(arr, 8);
    assert(arr[0] == 2 && arr[7] == 802);
    return 0;
}
