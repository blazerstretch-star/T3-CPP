#include "../src/jemalloc_utils.h"
#include <cassert>
#include <cstring>

using namespace JemallocUtils;

int main() {
    bitmap_info_t binfo;
    bitmap_info_init(&binfo, 64);
    bitmap_t bitmap[1];
    
    // PROPERTY 1: Fill=true creates full bitmap (all bits set)
    bitmap_init(bitmap, &binfo, true);
    assert(bitmap_full(bitmap, &binfo) && "Fill=true must create full bitmap");
    
    // Verify all bits are set
    for (size_t i = 0; i < 64; i++) {
        assert(bitmap_get(bitmap, &binfo, i) && "All bits must be set when fill=true");
    }
    
    // PROPERTY 2: Fill=false creates empty bitmap (all bits unset)
    bitmap_init(bitmap, &binfo, false);
    assert(!bitmap_full(bitmap, &binfo) && "Fill=false must create non-full bitmap");
    
    // Verify all bits are unset
    for (size_t i = 0; i < 64; i++) {
        assert(!bitmap_get(bitmap, &binfo, i) && "All bits must be unset when fill=false");
    }
    
    // PROPERTY 3: Multiple initializations work correctly
    bitmap_init(bitmap, &binfo, true);
    assert(bitmap_full(bitmap, &binfo));
    bitmap_init(bitmap, &binfo, false);
    assert(!bitmap_full(bitmap, &binfo));
    bitmap_init(bitmap, &binfo, true);
    assert(bitmap_full(bitmap, &binfo));
    
    // PROPERTY 4: Works with different bitmap sizes
    bitmap_info_t binfo2;
    bitmap_info_init(&binfo2, 128);
    bitmap_t bitmap2[2];
    
    bitmap_init(bitmap2, &binfo2, true);
    assert(bitmap_full(bitmap2, &binfo2));
    
    bitmap_init(bitmap2, &binfo2, false);
    assert(!bitmap_full(bitmap2, &binfo2));
    
    // PROPERTY 5: Non-multiple of 64 bits (65 bits)
    bitmap_info_t binfo3;
    bitmap_info_init(&binfo3, 65);
    bitmap_t bitmap3[2];
    
    bitmap_init(bitmap3, &binfo3, true);
    // All valid bits should be set
    for (size_t i = 0; i < 65; i++) {
        assert(bitmap_get(bitmap3, &binfo3, i) && "All valid bits must be set");
    }
    
    bitmap_init(bitmap3, &binfo3, false);
    // All valid bits should be unset
    for (size_t i = 0; i < 65; i++) {
        assert(!bitmap_get(bitmap3, &binfo3, i) && "All valid bits must be unset");
    }
    
    return 0;
}
