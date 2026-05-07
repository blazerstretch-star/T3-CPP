#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    assert(Algorithms::max(5, 3) == 5);
    assert(Algorithms::max(3, 5) == 5);
    assert(Algorithms::max(-1, -5) == -1);
    return 0;
}
