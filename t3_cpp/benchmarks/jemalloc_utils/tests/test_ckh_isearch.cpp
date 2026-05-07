#include "../src/jemalloc_utils.h"
#include <cassert>
#include <cstring>

using namespace JemallocUtils;

int main() {
    ckh_t ckh;
    ckh.lg_curbuckets = 4;
    ckh.prng_state = 12345;
    ckh.hash = ckh_pointer_hash;
    ckh.keycomp = ckh_pointer_keycomp;
    ckhc_t cells[256];
    memset(cells, 0, sizeof(cells));
    ckh.tab = cells;
    
    int key = 42;
    size_t idx = ckh_isearch(&ckh, &key);
    // Should return SIZE_MAX when not found
    assert(idx == SIZE_MAX);
    
    return 0;
}
