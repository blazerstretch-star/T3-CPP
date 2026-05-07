#include "../src/jemalloc_utils.h"
#include <cassert>

using namespace JemallocUtils;

int main() {
    ckh_t ckh;
    ckh.count = 42;
    assert(ckh_count(&ckh) == 42);
    
    ckh.count = 0;
    assert(ckh_count(&ckh) == 0);
    
    ckh.count = 1000;
    assert(ckh_count(&ckh) == 1000);
    
    return 0;
}
