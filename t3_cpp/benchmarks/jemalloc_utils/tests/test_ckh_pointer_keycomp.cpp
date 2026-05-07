#include "../src/jemalloc_utils.h"
#include <cassert>

using namespace JemallocUtils;

int main() {
    int x = 10, y = 20;
    assert(ckh_pointer_keycomp(&x, &x));
    assert(!ckh_pointer_keycomp(&x, &y));
    
    return 0;
}
