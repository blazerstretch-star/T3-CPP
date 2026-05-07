#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    int a = 5, b = 3;
    Algorithms::swap(a, b);
    assert(a == 3 && b == 5);
    return 0;
}
