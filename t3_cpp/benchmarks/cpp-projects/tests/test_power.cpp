#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    assert(Algorithms::power(2, 3) == 8);
    assert(Algorithms::power(5, 0) == 1);
    assert(Algorithms::power(3, 2) == 9);
    return 0;
}
