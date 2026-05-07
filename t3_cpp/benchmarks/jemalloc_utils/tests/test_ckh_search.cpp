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
    
    int key = 42, data = 100;
    
    // Calculate where key should go using the hash function
    size_t hashes[2];
    ckh_pointer_hash(&key, hashes);
    size_t bucket = hashes[0] & ((1U << ckh.lg_curbuckets) - 1);
    size_t idx = bucket << LG_CKH_BUCKET_CELLS;
    
    // Insert manually at correct location
    cells[idx].key = &key;
    cells[idx].data = &data;
    
    // PROPERTY 1: Find existing key with both key and data output
    void *found_key = nullptr, *found_data = nullptr;
    bool result = ckh_search(&ckh, &key, &found_key, &found_data);
    assert(!result && "Search should succeed (return false) for existing key");
    assert(found_key == &key && "Found key must match search key");
    assert(found_data == &data && "Found data must match inserted data");
    
    // PROPERTY 2: Find with NULL key pointer
    found_data = nullptr;
    result = ckh_search(&ckh, &key, nullptr, &found_data);
    assert(!result && "Search should work with NULL key pointer");
    assert(found_data == &data && "Data should still be returned");
    
    // PROPERTY 3: Find with NULL data pointer
    found_key = nullptr;
    result = ckh_search(&ckh, &key, &found_key, nullptr);
    assert(!result && "Search should work with NULL data pointer");
    assert(found_key == &key && "Key should still be returned");
    
    // PROPERTY 4: Find with both NULL pointers
    result = ckh_search(&ckh, &key, nullptr, nullptr);
    assert(!result && "Search should work with both NULL pointers");
    
    // PROPERTY 5: Not found returns true
    int missing_key = 999;
    result = ckh_search(&ckh, &missing_key, &found_key, &found_data);
    assert(result && "Search should fail (return true) for missing key");
    
    // PROPERTY 6: Search doesn't modify table
    result = ckh_search(&ckh, &key, &found_key, &found_data);
    assert(!result);
    // Search again - should still find it
    result = ckh_search(&ckh, &key, &found_key, &found_data);
    assert(!result && "Repeated search should succeed");
    assert(found_key == &key && "Key should still be found");
    
    // PROPERTY 7: Empty table search
    memset(cells, 0, sizeof(cells));
    result = ckh_search(&ckh, &key, &found_key, &found_data);
    assert(result && "Search in empty table should fail");
    
    // PROPERTY 8: Multiple keys in correct buckets
    memset(cells, 0, sizeof(cells));
    int key1 = 10, data1 = 100;
    int key2 = 20, data2 = 200;
    
    // Insert key1 in its correct bucket
    ckh_pointer_hash(&key1, hashes);
    bucket = hashes[0] & ((1U << ckh.lg_curbuckets) - 1);
    idx = bucket << LG_CKH_BUCKET_CELLS;
    cells[idx].key = &key1;
    cells[idx].data = &data1;
    
    // Insert key2 in its correct bucket
    ckh_pointer_hash(&key2, hashes);
    bucket = hashes[0] & ((1U << ckh.lg_curbuckets) - 1);
    idx = bucket << LG_CKH_BUCKET_CELLS;
    // If same bucket, use next cell
    if (cells[idx].key != nullptr) {
        idx++;
    }
    cells[idx].key = &key2;
    cells[idx].data = &data2;
    
    // Search for both keys
    result = ckh_search(&ckh, &key1, &found_key, &found_data);
    assert(!result && "Should find key1");
    assert(found_key == &key1 && found_data == &data1);
    
    result = ckh_search(&ckh, &key2, &found_key, &found_data);
    assert(!result && "Should find key2");
    assert(found_key == &key2 && found_data == &data2);
    
    return 0;
}
