#include "../src/jemalloc_utils.h"
#include <cassert>
#include <cstring>

using namespace JemallocUtils;

int main() {
    ckh_t ckh;
    ckh.lg_curbuckets = 4;
    ckh.count = 5;
    ckh.hash = ckh_pointer_hash;
    ckh.keycomp = ckh_pointer_keycomp;
    
    ckhc_t old_tab[256];
    memset(old_tab, 0, sizeof(old_tab));
    
    // Add some items to old table
    int keys[5] = {10, 20, 30, 40, 50};
    int datas[5] = {100, 200, 300, 400, 500};
    for (int i = 0; i < 5; i++) {
        old_tab[i * 10].key = &keys[i];
        old_tab[i * 10].data = &datas[i];
    }
    
    ckhc_t new_tab[256];
    memset(new_tab, 0, sizeof(new_tab));
    ckh.tab = new_tab;
    
    size_t old_count = ckh.count;
    ckh_rebuild(&ckh, old_tab);
    
    // Count should be preserved
    assert(ckh.count == old_count);
    
    // All items should be in new table
    int found_count = 0;
    for (size_t i = 0; i < 256; i++) {
        if (new_tab[i].key != NULL) {
            found_count++;
            // Verify it's one of our keys
            bool valid = false;
            for (int j = 0; j < 5; j++) {
                if (new_tab[i].key == &keys[j]) {
                    assert(new_tab[i].data == &datas[j]);
                    valid = true;
                    break;
                }
            }
            assert(valid);
        }
    }
    assert(found_count == 5);
    
    return 0;
}
