#include "../../src/jemalloc_utils.h"
#include <cstring>
#include <cassert>

#define ZU(x) ((size_t)(x))

static inline unsigned ffs_lu(unsigned long x) {
    return __builtin_ffsl(x) - 1;
}

namespace JemallocUtils {

void bitmap_info_init(bitmap_info_t *binfo, size_t nbits) {
    // FUNCTION_ID: jemalloc_func001 - START
    assert(nbits > 0);
    binfo->ngroups = BITMAP_BITS2GROUPS(nbits);
    binfo->nbits = nbits;
    // FUNCTION_ID: jemalloc_func001 - END
}

size_t bitmap_size(const bitmap_info_t *binfo) {
    // FUNCTION_ID: jemalloc_func002 - START
    return (binfo->ngroups << LG_SIZEOF_BITMAP);
    // FUNCTION_ID: jemalloc_func002 - END
}

bool bitmap_get(bitmap_t *bitmap, const bitmap_info_t *binfo, size_t bit) {
    // FUNCTION_ID: jemalloc_func003 - START
    size_t goff = bit >> LG_BITMAP_GROUP_NBITS;
    size_t gp = bit & BITMAP_GROUP_NBITS_MASK;
    return !(bitmap[goff] & (ZU(1) << gp));
    // FUNCTION_ID: jemalloc_func003 - END
}

void bitmap_set(bitmap_t *bitmap, const bitmap_info_t *binfo, size_t bit) {
    // FUNCTION_ID: jemalloc_func004 - START
    size_t goff = bit >> LG_BITMAP_GROUP_NBITS;
    size_t gp = bit & BITMAP_GROUP_NBITS_MASK;
    bitmap_t g = bitmap[goff];
    bitmap_t b = ZU(1) << gp;
    assert(g & b);
    g ^= b;
    bitmap[goff] = g;
    // FUNCTION_ID: jemalloc_func004 - END
}

void bitmap_init(bitmap_t *bitmap, const bitmap_info_t *binfo, bool fill) {
    // FUNCTION_ID: jemalloc_func009 - START
    size_t extra;
    
    if (fill) {
        memset(bitmap, 0, bitmap_size(binfo));
        return;
    }
    
    memset(bitmap, 0xffU, bitmap_size(binfo));
    extra = (BITMAP_GROUP_NBITS - (binfo->nbits & BITMAP_GROUP_NBITS_MASK))
        & BITMAP_GROUP_NBITS_MASK;
    if (extra != 0) {
        bitmap[binfo->ngroups - 1] >>= extra;
    }
    // FUNCTION_ID: jemalloc_func009 - END
}

void bitmap_unset(bitmap_t *bitmap, const bitmap_info_t *binfo, size_t bit) {
    // FUNCTION_ID: jemalloc_func010 - START
    size_t goff = bit >> LG_BITMAP_GROUP_NBITS;
    size_t gp = bit & BITMAP_GROUP_NBITS_MASK;
    bitmap_t g = bitmap[goff];
    bitmap_t b = ZU(1) << gp;
    assert(!(g & b));
    g ^= b;
    bitmap[goff] = g;
    // FUNCTION_ID: jemalloc_func010 - END
}

bool bitmap_full(bitmap_t *bitmap, const bitmap_info_t *binfo) {
    // FUNCTION_ID: jemalloc_func011 - START
    for (size_t i = 0; i < binfo->ngroups; i++) {
        if (bitmap[i] != 0) {
            return false;
        }
    }
    return true;
    // FUNCTION_ID: jemalloc_func011 - END
}

size_t bitmap_ffu(const bitmap_t *bitmap, const bitmap_info_t *binfo, size_t min_bit) {
    // FUNCTION_ID: jemalloc_func012 - START
    size_t goff = min_bit >> LG_BITMAP_GROUP_NBITS;
    size_t gp = min_bit & BITMAP_GROUP_NBITS_MASK;
    
    bitmap_t g = bitmap[goff] & ~((ZU(1) << gp) - 1);
    if (g != 0) {
        return (goff << LG_BITMAP_GROUP_NBITS) + ffs_lu(g);
    }
    
    for (goff++; goff < binfo->ngroups; goff++) {
        g = bitmap[goff];
        if (g != 0) {
            return (goff << LG_BITMAP_GROUP_NBITS) + ffs_lu(g);
        }
    }
    
    return binfo->nbits;
    // FUNCTION_ID: jemalloc_func012 - END
}

size_t bitmap_sfu(bitmap_t *bitmap, const bitmap_info_t *binfo) {
    // FUNCTION_ID: jemalloc_func013 - START
    for (size_t i = 0; i < binfo->ngroups; i++) {
        bitmap_t g = bitmap[i];
        if (g != 0) {
            size_t bit = (i << LG_BITMAP_GROUP_NBITS) + ffs_lu(g);
            if (bit < binfo->nbits) {
                bitmap_set(bitmap, binfo, bit);
                return bit;
            }
        }
    }
    return binfo->nbits;
    // FUNCTION_ID: jemalloc_func013 - END
}

} // namespace JemallocUtils
