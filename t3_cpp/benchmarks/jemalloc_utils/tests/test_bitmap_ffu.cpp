#include "../src/jemalloc_utils.h"
#include <cassert>
#include <cstring>

using namespace JemallocUtils;

int main() {
    bitmap_info_t binfo;
    bitmap_info_init(&binfo, 64);
    bitmap_t bitmap[1];
    
    // PROPERTY 1: Find first unset bit from beginning
    memset(bitmap, 0, sizeof(bitmap));  // All 0s = all set
    bitmap_unset(bitmap, &binfo, 10);  // Unset bit 10
    size_t bit = bitmap_ffu(bitmap, &binfo, 0);
    assert(bit == 10 && "Should find first unset bit at position 10");
    
    // PROPERTY 2: Find from specific starting position
    bitmap_unset(bitmap, &binfo, 5);   // Unset bit 5
    bit = bitmap_ffu(bitmap, &binfo, 8);  // Start search from bit 8
    assert(bit == 10 && "Should skip bit 5 and find bit 10");
    
    // PROPERTY 3: Find from exact position of unset bit
    bit = bitmap_ffu(bitmap, &binfo, 10);
    assert(bit == 10 && "Should find bit when starting at that position");
    
    // PROPERTY 4: No unset bits found returns nbits
    memset(bitmap, 0, sizeof(bitmap));  // All set
    bit = bitmap_ffu(bitmap, &binfo, 0);
    assert(bit == binfo.nbits && "Should return nbits when no unset bits found");
    
    // PROPERTY 5: Multiple unset bits - finds first
    memset(bitmap, 0, sizeof(bitmap));
    bitmap_unset(bitmap, &binfo, 20);
    bitmap_unset(bitmap, &binfo, 30);
    bitmap_unset(bitmap, &binfo, 40);
    bit = bitmap_ffu(bitmap, &binfo, 0);
    assert(bit == 20 && "Should find first unset bit");
    
    // PROPERTY 6: Search from middle finds next unset
    bit = bitmap_ffu(bitmap, &binfo, 25);
    assert(bit == 30 && "Should find next unset bit after start position");
    
    // PROPERTY 7: Search past all unset bits
    bit = bitmap_ffu(bitmap, &binfo, 50);
    assert(bit == binfo.nbits && "Should return nbits when starting past all unset bits");
    
    // PROPERTY 8: First bit unset
    memset(bitmap, 0, sizeof(bitmap));
    bitmap_unset(bitmap, &binfo, 0);
    bit = bitmap_ffu(bitmap, &binfo, 0);
    assert(bit == 0 && "Should find bit 0 when it's unset");
    
    // PROPERTY 9: Last bit unset
    memset(bitmap, 0, sizeof(bitmap));
    bitmap_unset(bitmap, &binfo, 63);
    bit = bitmap_ffu(bitmap, &binfo, 0);
    assert(bit == 63 && "Should find last bit when it's unset");
    
    // PROPERTY 10: All bits unset - finds first
    memset(bitmap, 0xFF, sizeof(bitmap));  // All unset
    bit = bitmap_ffu(bitmap, &binfo, 0);
    assert(bit == 0 && "Should find bit 0 when all bits unset");
    
    return 0;
}
