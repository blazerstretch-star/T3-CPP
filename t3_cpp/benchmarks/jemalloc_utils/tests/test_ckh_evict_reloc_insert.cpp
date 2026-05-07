#include "../src/jemalloc_utils.h"
#include <cassert>
#include <cstring>

using namespace JemallocUtils;

int main() {
    ckh_t ckh;
    ckh.lg_curbuckets = 2;
    ckh.prng_state = 12345;
    ckh.count = 0;
    ckhc_t cells[32];
    memset(cells, 0, sizeof(cells));
    ckh.tab = cells;
    ckh.hash = ckh_pointer_hash;
    ckh.keycomp = ckh_pointer_keycomp;
    
    // Fill bucket 0 completely (8 cells)
    for (int i = 0; i < 8; i++) {
        cells[i].key = (void*)(intptr_t)(i + 1);
        cells[i].data = (void*)(intptr_t)(i + 100);
    }
    
    int key = 999, data = 888;
    bool result = ckh_evict_reloc_insert(&ckh, 0, &key, &data);
    
    // Should successfully relocate (false = success)
    assert(!result);
    
    // Verify key was inserted somewhere
    bool found = false;
    for (int i = 0; i < 32; i++) {
        if (cells[i].key == &key) {
            found = true;
            assert(cells[i].data == &data);
            break;
        }
    }
    assert(found);
    
    // Verify at least one item was evicted from bucket 0
    int bucket0_count = 0;
    for (int i = 0; i < 8; i++) {
        if (cells[i].key != NULL) bucket0_count++;
    }
    assert(bucket0_count == 8);
    
    return 0;
}
