#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    int pos = Algorithms::hashDoubleHash(10, 7, 0);
    assert(pos >= 0 && pos < 7);
    return 0;
}
