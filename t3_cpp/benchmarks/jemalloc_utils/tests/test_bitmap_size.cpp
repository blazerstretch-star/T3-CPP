#include "../src/jemalloc_utils.h"
#include <cassert>

using namespace JemallocUtils;

int main() {
    bitmap_info_t binfo;
    bitmap_info_init(&binfo, 64);
    size_t size = bitmap_size(&binfo);
    assert(size == sizeof(bitmap_t));
    
    bitmap_info_init(&binfo, 128);
    size = bitmap_size(&binfo);
    assert(size == 2 * sizeof(bitmap_t));
    
    return 0;
}
