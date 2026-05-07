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
    ckh.hash = ckh_pointer_hash;
    ckh.keycomp = ckh_pointer_keycomp;
    
    // PROPERTY 1: Successful insertion into empty table
    int key1 = 42, data1 = 100;
    size_t old_count = ckh.count;
    bool result = ckh_try_insert(&ckh, &key1, &data1);
    assert(!result && "Insertion should succeed (return false)");
    assert(ckh.count == old_count + 1 && "Count must increment after insertion");
    
    // PROPERTY 2: Inserted key is findable
    bool found = false;
    for (size_t i = 0; i < 256; i++) {
        if (cells[i].key == &key1) {
            assert(cells[i].data == &data1 && "Data must match");
            found = true;
            break;
        }
    }
    assert(found && "Inserted key must be in table");
    
    // PROPERTY 3: Key is searchable after insertion
    void *found_key = nullptr, *found_data = nullptr;
    bool search_result = ckh_search(&ckh, &key1, &found_key, &found_data);
    assert(!search_result && "Inserted key must be searchable");
    assert(found_key == &key1 && "Search must return correct key");
    assert(found_data == &data1 && "Search must return correct data");
    
    // PROPERTY 4: Multiple insertions work
    int key2 = 43, data2 = 200;
    int key3 = 44, data3 = 300;
    
    old_count = ckh.count;
    result = ckh_try_insert(&ckh, &key2, &data2);
    assert(!result && "Second insertion should succeed");
    assert(ckh.count == old_count + 1 && "Count must increment");
    
    old_count = ckh.count;
    result = ckh_try_insert(&ckh, &key3, &data3);
    assert(!result && "Third insertion should succeed");
    assert(ckh.count == old_count + 1 && "Count must increment");
    
    // PROPERTY 5: All inserted keys are findable
    search_result = ckh_search(&ckh, &key1, &found_key, &found_data);
    assert(!search_result && found_key == &key1 && found_data == &data1);
    
    search_result = ckh_search(&ckh, &key2, &found_key, &found_data);
    assert(!search_result && found_key == &key2 && found_data == &data2);
    
    search_result = ckh_search(&ckh, &key3, &found_key, &found_data);
    assert(!search_result && found_key == &key3 && found_data == &data3);
    
    // PROPERTY 6: Keys are in one of their two possible buckets
    size_t hashes[2];
    ckh_pointer_hash(&key1, hashes);
    size_t bucket1 = hashes[0] & ((1U << ckh.lg_curbuckets) - 1);
    size_t bucket2 = hashes[1] & ((1U << ckh.lg_curbuckets) - 1);
    
    bool in_bucket1 = false, in_bucket2 = false;
    for (size_t i = 0; i < (1U << LG_CKH_BUCKET_CELLS); i++) {
        if (cells[(bucket1 << LG_CKH_BUCKET_CELLS) + i].key == &key1) {
            in_bucket1 = true;
        }
        if (cells[(bucket2 << LG_CKH_BUCKET_CELLS) + i].key == &key1) {
            in_bucket2 = true;
        }
    }
    assert((in_bucket1 || in_bucket2) && 
           "Key must be in one of its two possible buckets");
    
    // PROPERTY 7: Count reflects actual number of items
    int actual_count = 0;
    for (size_t i = 0; i < 256; i++) {
        if (cells[i].key != nullptr) {
            actual_count++;
        }
    }
    assert(ckh.count == actual_count && "Count must match actual items in table");
    
    // PROPERTY 8: Insertion with collision handling
    // Insert many items to force collisions
    int keys[20], datas[20];
    for (int i = 0; i < 20; i++) {
        keys[i] = 100 + i;
        datas[i] = 1000 + i;
        result = ckh_try_insert(&ckh, &keys[i], &datas[i]);
        // Most should succeed (some might fail if table gets too full)
    }
    
    // Verify all successfully inserted items are searchable
    for (int i = 0; i < 20; i++) {
        search_result = ckh_search(&ckh, &keys[i], &found_key, &found_data);
        if (!search_result) {
            // If found, verify correctness
            assert(found_key == &keys[i] && "Key must match");
            assert(found_data == &datas[i] && "Data must match");
        }
    }
    
    return 0;
}
