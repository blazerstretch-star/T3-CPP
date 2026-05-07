#include "../src/jemalloc_utils.h"
#include <cassert>
#include <cstring>

using namespace JemallocUtils;

int main() {
    bitmap_info_t binfo;
    bitmap_info_init(&binfo, 64);
    bitmap_t bitmap[1];
    
    // PROPERTY 1: Find and set first unset bit
    memset(bitmap, 0, sizeof(bitmap));  // All 0s = all set
    bitmap_unset(bitmap, &binfo, 10);
    size_t bit = bitmap_sfu(bitmap, &binfo);
    assert(bit == 10 && "Should find and return bit 10");
    assert(bitmap_get(bitmap, &binfo, bit) && "Bit must be set after sfu");
    
    // PROPERTY 2: Subsequent call finds next unset bit
    bitmap_unset(bitmap, &binfo, 20);
    bit = bitmap_sfu(bitmap, &binfo);
    assert(bit == 20 && "Should find next unset bit");
    assert(bitmap_get(bitmap, &binfo, bit) && "Bit must be set after sfu");
    
    // PROPERTY 3: All bits set returns nbits
    memset(bitmap, 0, sizeof(bitmap));  // All set
    bit = bitmap_sfu(bitmap, &binfo);
    assert(bit == binfo.nbits && "Should return nbits when all bits set");
    
    // PROPERTY 4: Multiple unset bits - processes in order
    memset(bitmap, 0, sizeof(bitmap));
    bitmap_unset(bitmap, &binfo, 5);
    bitmap_unset(bitmap, &binfo, 15);
    bitmap_unset(bitmap, &binfo, 25);
    
    bit = bitmap_sfu(bitmap, &binfo);
    assert(bit == 5 && "Should find first unset bit");
    assert(bitmap_get(bitmap, &binfo, 5) && "Bit 5 must be set");
    assert(!bitmap_get(bitmap, &binfo, 15) && "Bit 15 should still be unset");
    
    bit = bitmap_sfu(bitmap, &binfo);
    assert(bit == 15 && "Should find second unset bit");
    assert(bitmap_get(bitmap, &binfo, 15) && "Bit 15 must be set");
    assert(!bitmap_get(bitmap, &binfo, 25) && "Bit 25 should still be unset");
    
    bit = bitmap_sfu(bitmap, &binfo);
    assert(bit == 25 && "Should find third unset bit");
    assert(bitmap_get(bitmap, &binfo, 25) && "Bit 25 must be set");
    
    // PROPERTY 5: After setting all, returns nbits
    bit = bitmap_sfu(bitmap, &binfo);
    assert(bit == binfo.nbits && "Should return nbits when all processed");
    
    // PROPERTY 6: First bit (bit 0) handling
    memset(bitmap, 0, sizeof(bitmap));
    bitmap_unset(bitmap, &binfo, 0);
    bit = bitmap_sfu(bitmap, &binfo);
    assert(bit == 0 && "Should handle bit 0 correctly");
    assert(bitmap_get(bitmap, &binfo, 0) && "Bit 0 must be set");
    
    // PROPERTY 7: Last bit (bit 63) handling
    memset(bitmap, 0, sizeof(bitmap));
    bitmap_unset(bitmap, &binfo, 63);
    bit = bitmap_sfu(bitmap, &binfo);
    assert(bit == 63 && "Should handle last bit correctly");
    assert(bitmap_get(bitmap, &binfo, 63) && "Last bit must be set");
    
    // PROPERTY 8: All unset - processes first
    memset(bitmap, 0xFF, sizeof(bitmap));  // All unset
    bit = bitmap_sfu(bitmap, &binfo);
    assert(bit == 0 && "Should find bit 0 when all unset");
    assert(bitmap_get(bitmap, &binfo, 0) && "Bit 0 must be set");
    assert(!bitmap_get(bitmap, &binfo, 1) && "Other bits should remain unset");
    
    // PROPERTY 9: State consistency - bit remains set
    memset(bitmap, 0, sizeof(bitmap));
    bitmap_unset(bitmap, &binfo, 30);
    bit = bitmap_sfu(bitmap, &binfo);
    assert(bit == 30);
    // Verify bit stays set
    assert(bitmap_get(bitmap, &binfo, 30) && "Bit must remain set");
    bit = bitmap_sfu(bitmap, &binfo);
    assert(bit == binfo.nbits && "Should not find bit 30 again");
    
    return 0;
}
