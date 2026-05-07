#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    assert(Algorithms::gcd(48, 18) == 6);
    assert(Algorithms::gcd(7, 5) == 1);
    assert(Algorithms::gcd(100, 50) == 50);
    return 0;
}
