#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    int arr[] = {5, 2, 8, 1, 9};
    assert(Algorithms::linearSearch(arr, 5, 8) == 2);
    assert(Algorithms::linearSearch(arr, 5, 10) == -1);
    return 0;
}
