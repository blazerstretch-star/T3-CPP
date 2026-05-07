#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    assert(Algorithms::absolute(5) == 5);
    assert(Algorithms::absolute(-5) == 5);
    assert(Algorithms::absolute(0) == 0);
    return 0;
}
