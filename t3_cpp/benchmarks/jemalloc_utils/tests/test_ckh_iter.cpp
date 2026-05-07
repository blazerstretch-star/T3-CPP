#include "../src/jemalloc_utils.h"
#include <cassert>
#include <cstring>

using namespace JemallocUtils;

int main() {
    ckh_t ckh;
    ckh.lg_curbuckets = 4;
    ckhc_t cells[256];
    memset(cells, 0, sizeof(cells));
    ckh.tab = cells;
    
    // PROPERTY 1: Empty table iteration
    size_t tabind = 0;
    void *found_key = nullptr, *found_data = nullptr;
    bool done = ckh_iter(&ckh, &tabind, &found_key, &found_data);
    assert(done && "Iteration on empty table should return true (done)");
    
    // PROPERTY 2: Single item iteration
    int key1 = 42, data1 = 100;
    cells[10].key = &key1;
    cells[10].data = &data1;
    
    tabind = 0;
    done = ckh_iter(&ckh, &tabind, &found_key, &found_data);
    assert(!done && "Should find first item");
    assert(found_key == &key1 && "Key must match inserted key");
    assert(found_data == &data1 && "Data must match inserted data");
    assert(tabind == 11 && "Index must advance past found item");
    
    // Continue iteration - should be done
    done = ckh_iter(&ckh, &tabind, &found_key, &found_data);
    assert(done && "Should be done after finding all items");
    
    // PROPERTY 3: Multiple items iteration
    int key2 = 43, data2 = 200;
    int key3 = 44, data3 = 300;
    cells[20].key = &key2;
    cells[20].data = &data2;
    cells[50].key = &key3;
    cells[50].data = &data3;
    
    // Iterate through all items
    tabind = 0;
    int found_count = 0;
    bool found_keys[3] = {false, false, false};
    
    while (!ckh_iter(&ckh, &tabind, &found_key, &found_data)) {
        found_count++;
        if (found_key == &key1 && found_data == &data1) found_keys[0] = true;
        if (found_key == &key2 && found_data == &data2) found_keys[1] = true;
        if (found_key == &key3 && found_data == &data3) found_keys[2] = true;
    }
    
    assert(found_count == 3 && "Must find exactly 3 items");
    assert(found_keys[0] && found_keys[1] && found_keys[2] && 
           "Must find all inserted keys");
    
    // PROPERTY 4: Resume iteration from specific index
    tabind = 25;  // Start after key2
    found_count = 0;
    while (!ckh_iter(&ckh, &tabind, &found_key, &found_data)) {
        found_count++;
    }
    assert(found_count == 1 && "Should only find items after index 25");
    
    // PROPERTY 5: NULL pointer handling (optional parameters)
    tabind = 0;
    done = ckh_iter(&ckh, &tabind, nullptr, nullptr);
    assert(!done && "Should work with NULL key/data pointers");
    
    return 0;
}
