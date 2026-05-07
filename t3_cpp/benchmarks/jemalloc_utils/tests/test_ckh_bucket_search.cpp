#include "../src/jemalloc_utils.h"
#include <cassert>
#include <cstring>

using namespace JemallocUtils;

int main() {
    ckh_t ckh;
    ckh.lg_curbuckets = 4;
    ckh.hash = ckh_pointer_hash;
    ckh.keycomp = ckh_pointer_keycomp;
    ckhc_t cells[256];
    memset(cells, 0, sizeof(cells));
    ckh.tab = cells;
    
    int key = 42;
    cells[0].key = &key;
    
    assert(ckh_bucket_search(&ckh, 0, &key));
    assert(!ckh_bucket_search(&ckh, 1, &key));
    
    return 0;
}
