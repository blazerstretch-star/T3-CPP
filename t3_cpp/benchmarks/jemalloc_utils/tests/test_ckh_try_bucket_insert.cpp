#include "../src/jemalloc_utils.h"
#include <cassert>
#include <cstring>

using namespace JemallocUtils;

int main() {
    ckh_t ckh;
    ckh.lg_curbuckets = 4;
    ckh.prng_state = 12345;
    ckhc_t cells[256];
    memset(cells, 0, sizeof(cells));
    ckh.tab = cells;
    ckh.count = 0;
    
    // PROPERTY 1: Successful insertion into empty bucket
    int key1 = 42, data1 = 100;
    bool result = ckh_try_bucket_insert(&ckh, 0, &key1, &data1);
    assert(!result && "Insertion into empty bucket should succeed (return false)");
    
    // PROPERTY 2: Count incremented after insertion
    assert(ckh.count == 1 && "Count must be incremented after insertion");
    
    // PROPERTY 3: Key and data actually stored in bucket 0
    bool found = false;
    size_t bucket_start = 0 << LG_CKH_BUCKET_CELLS;
    size_t bucket_end = bucket_start + (1U << LG_CKH_BUCKET_CELLS);
    for (size_t i = bucket_start; i < bucket_end; i++) {
        if (cells[i].key == &key1) {
            assert(cells[i].data == &data1 && "Data must match inserted data");
            found = true;
            break;
        }
    }
    assert(found && "Key must be stored in the specified bucket");
    
    // PROPERTY 4: Multiple insertions into same bucket
    int key2 = 43, data2 = 200;
    size_t old_count = ckh.count;
    result = ckh_try_bucket_insert(&ckh, 0, &key2, &data2);
    assert(!result && "Second insertion should succeed");
    assert(ckh.count == old_count + 1 && "Count must increment for each insertion");
    
    // PROPERTY 5: Full bucket detection (8 cells per bucket)
    // Fill remaining cells in bucket 0
    int keys[8], datas[8];
    for (int i = 2; i < 8; i++) {
        keys[i] = 50 + i;
        datas[i] = 300 + i;
        ckh_try_bucket_insert(&ckh, 0, &keys[i], &datas[i]);
    }
    
    // Try to insert into full bucket
    int key_overflow = 999, data_overflow = 888;
    result = ckh_try_bucket_insert(&ckh, 0, &key_overflow, &data_overflow);
    assert(result && "Insertion into full bucket should fail (return true)");
    
    // PROPERTY 6: Failed insertion doesn't increment count
    assert(ckh.count == 8 && "Count should not change on failed insertion");
    
    return 0;
}
