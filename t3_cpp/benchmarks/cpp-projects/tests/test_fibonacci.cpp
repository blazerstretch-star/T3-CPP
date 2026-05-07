#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    assert(Algorithms::fibonacci(0) == 0);
    assert(Algorithms::fibonacci(1) == 1);
    assert(Algorithms::fibonacci(6) == 8);
    return 0;
}
