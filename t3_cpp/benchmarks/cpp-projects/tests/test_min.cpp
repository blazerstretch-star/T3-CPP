#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    assert(Algorithms::min(5, 3) == 3);
    assert(Algorithms::min(3, 5) == 3);
    assert(Algorithms::min(-1, -5) == -5);
    return 0;
}
