#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    assert(Algorithms::factorial(5) == 120);
    assert(Algorithms::factorial(0) == 1);
    assert(Algorithms::factorial(3) == 6);
    return 0;
}
