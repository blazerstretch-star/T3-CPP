#include "../src/jemalloc_utils.h"
#include <cassert>
#include <cstring>

using namespace JemallocUtils;

int main() {
    bitmap_info_t binfo;
    bitmap_info_init(&binfo, 64);
    bitmap_t bitmap[1];
    memset(bitmap, 0xFF, sizeof(bitmap));  // All 1s = all unset
    
    assert(!bitmap_get(bitmap, &binfo, 0));  // 1 = unset, so get returns false
    assert(!bitmap_get(bitmap, &binfo, 10));
    
    bitmap[0] = ~(1UL << 5);  // Clear bit 5 = set bit 5
    assert(bitmap_get(bitmap, &binfo, 5));  // 0 = set, so get returns true
    assert(!bitmap_get(bitmap, &binfo, 6));
    
    return 0;
}
