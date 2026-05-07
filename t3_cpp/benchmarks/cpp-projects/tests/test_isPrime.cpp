#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    assert(Algorithms::isPrime(7) == true);
    assert(Algorithms::isPrime(4) == false);
    assert(Algorithms::isPrime(2) == true);
    return 0;
}
