#include "../src/jemalloc_utils.h"
#include <cassert>
#include <cstring>

using namespace JemallocUtils;

int main() {
    bitmap_info_t binfo;
    bitmap_info_init(&binfo, 64);
    bitmap_t bitmap[1];
    
    memset(bitmap, 0, sizeof(bitmap));
    assert(bitmap_full(bitmap, &binfo));
    
    bitmap[0] = 1;
    assert(!bitmap_full(bitmap, &binfo));
    
    return 0;
}
