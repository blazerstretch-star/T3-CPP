#include "../src/jemalloc_utils.h"
#include <cassert>
#include <cstring>

using namespace JemallocUtils;

int main() {
    bitmap_info_t binfo;
    bitmap_info_init(&binfo, 64);
    bitmap_t bitmap[1];
    memset(bitmap, 0xFF, sizeof(bitmap));  // All 1s = all unset
    
    bitmap_set(bitmap, &binfo, 5);
    assert(bitmap_get(bitmap, &binfo, 5));
    bitmap_unset(bitmap, &binfo, 5);
    assert(!bitmap_get(bitmap, &binfo, 5));
    
    return 0;
}
