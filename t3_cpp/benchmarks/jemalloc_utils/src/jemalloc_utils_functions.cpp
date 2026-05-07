#include "jemalloc_utils.h"
// Standard C++ Library - Complete Set for Algorithmic Reasoning
#include <algorithm>
#include <numeric>
#include <cmath>
#include <cstring>
#include <cassert>
#include <cstdint>
#include <climits>
#include <limits>
// Containers
#include <vector>
#include <array>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <stack>
// Utilities
#include <string>
#include <utility>
#include <functional>
#include <memory>
#include <tuple>
#include <bitset>
// Standard C++ Library (pre-included for agent evaluation)
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <queue>
#include <stack>
#include <deque>
#include <list>
#include <algorithm>
#include <cmath>
#include <string>
#include <memory>
#include <utility>


#define ZU(x) ((size_t)(x))
#define UNUSED __attribute__((unused))

static inline unsigned ffs_lu(unsigned long x) {
    return __builtin_ffsl(x) - 1;
}

static void hash(const void *key, size_t len, uint32_t seed, size_t r_hash[2]) {
    const uint8_t *data = (const uint8_t *)key;
    uint32_t h1 = seed;
    uint32_t h2 = seed;
    
    for (size_t i = 0; i < len; i++) {
        h1 = h1 * 31 + data[i];
        h2 = h2 * 37 + data[i];
    }
    
    r_hash[0] = h1;
    r_hash[1] = h2;
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

size_t ckh_count(ckh_t *ckh) {
    // FUNCTION_ID: jemalloc_func005 - START
    return ckh->count;
    // FUNCTION_ID: jemalloc_func005 - END
}

void ckh_pointer_hash(const void *key, size_t r_hash[2]) {
    // FUNCTION_ID: jemalloc_func006 - START
    union {
        const void *v;
        size_t i;
    } u;
    u.v = key;
    hash(&u.i, sizeof(u.i), 0xd983396eU, r_hash);
    // FUNCTION_ID: jemalloc_func006 - END
}

bool ckh_pointer_keycomp(const void *k1, const void *k2) {
    // FUNCTION_ID: jemalloc_func007 - START
    return (k1 == k2);
    // FUNCTION_ID: jemalloc_func007 - END
}

bool ckh_string_keycomp(const void *k1, const void *k2) {
    // FUNCTION_ID: jemalloc_func008 - START
    return !strcmp((char *)k1, (char *)k2);
    // FUNCTION_ID: jemalloc_func008 - END
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

void ckh_string_hash(const void *key, size_t r_hash[2]) {
    // FUNCTION_ID: jemalloc_func014 - START
    hash(key, strlen((const char *)key), 0x94122f33U, r_hash);
    // FUNCTION_ID: jemalloc_func014 - END
}

bool ckh_bucket_search(ckh_t *ckh, size_t bucket, const void *key) {
    // FUNCTION_ID: jemalloc_func015 - START
    ckhc_t *cell;
    for (unsigned i = 0; i < (ZU(1) << LG_CKH_BUCKET_CELLS); i++) {
        cell = &ckh->tab[(bucket << LG_CKH_BUCKET_CELLS) + i];
        if (cell->key != NULL && ckh->keycomp(key, cell->key)) {
            return true;
        }
    }
    return false;
    // FUNCTION_ID: jemalloc_func015 - END
}

size_t ckh_isearch(ckh_t *ckh, const void *key) {
    // FUNCTION_ID: jemalloc_func016 - START
    size_t hashes[2], bucket;
    
    ckh->hash(key, hashes);
    
    bucket = hashes[0] & ((ZU(1) << ckh->lg_curbuckets) - 1);
    if (ckh_bucket_search(ckh, bucket, key)) {
        for (unsigned i = 0; i < (ZU(1) << LG_CKH_BUCKET_CELLS); i++) {
            ckhc_t *cell = &ckh->tab[(bucket << LG_CKH_BUCKET_CELLS) + i];
            if (cell->key != NULL && ckh->keycomp(key, cell->key)) {
                return (bucket << LG_CKH_BUCKET_CELLS) + i;
            }
        }
    }
    
    bucket = hashes[1] & ((ZU(1) << ckh->lg_curbuckets) - 1);
    if (ckh_bucket_search(ckh, bucket, key)) {
        for (unsigned i = 0; i < (ZU(1) << LG_CKH_BUCKET_CELLS); i++) {
            ckhc_t *cell = &ckh->tab[(bucket << LG_CKH_BUCKET_CELLS) + i];
            if (cell->key != NULL && ckh->keycomp(key, cell->key)) {
                return (bucket << LG_CKH_BUCKET_CELLS) + i;
            }
        }
    }
    
    return SIZE_MAX;
    // FUNCTION_ID: jemalloc_func016 - END
}

bool ckh_try_bucket_insert(ckh_t *ckh, size_t bucket, const void *key, const void *data) {
    // FUNCTION_ID: jemalloc_func017 - START
    ckhc_t *cell;
    unsigned offset = (unsigned)(ckh->prng_state % (ZU(1) << LG_CKH_BUCKET_CELLS));
    
    for (unsigned i = 0; i < (ZU(1) << LG_CKH_BUCKET_CELLS); i++) {
        cell = &ckh->tab[(bucket << LG_CKH_BUCKET_CELLS) +
            ((i + offset) & ((ZU(1) << LG_CKH_BUCKET_CELLS) - 1))];
        if (cell->key == NULL) {
            cell->key = key;
            cell->data = data;
            ckh->count++;
            return false;
        }
    }
    
    return true;
    // FUNCTION_ID: jemalloc_func017 - END
}

bool ckh_search(ckh_t *ckh, const void *searchkey, void **key, void **data) {
    // FUNCTION_ID: jemalloc_func018 - START
    size_t cell = ckh_isearch(ckh, searchkey);
    
    if (cell != SIZE_MAX) {
        if (key != NULL) {
            *key = (void *)ckh->tab[cell].key;
        }
        if (data != NULL) {
            *data = (void *)ckh->tab[cell].data;
        }
        return false;
    }
    
    return true;
    // FUNCTION_ID: jemalloc_func018 - END
}

bool ckh_iter(ckh_t *ckh, size_t *tabind, void **key, void **data) {
    // FUNCTION_ID: jemalloc_func019 - START
    size_t ncells = (ZU(1) << (ckh->lg_curbuckets + LG_CKH_BUCKET_CELLS));
    
    for (size_t i = *tabind; i < ncells; i++) {
        if (ckh->tab[i].key != NULL) {
            if (key != NULL) {
                *key = (void *)ckh->tab[i].key;
            }
            if (data != NULL) {
                *data = (void *)ckh->tab[i].data;
            }
            *tabind = i + 1;
            return false;
        }
    }
    
    return true;
    // FUNCTION_ID: jemalloc_func019 - END
}

uint64_t ckh_prng(uint64_t *state) {
    // FUNCTION_ID: jemalloc_func020 - START
    *state = (*state * 6364136223846793005ULL) + 1442695040888963407ULL;
    return *state;
    // FUNCTION_ID: jemalloc_func020 - END
}

bool ckh_try_insert(ckh_t *ckh, const void *key, const void *data) {
    // FUNCTION_ID: jemalloc_func021 - START
    size_t hashes[2], bucket;
    
    ckh->hash(key, hashes);
    
    bucket = hashes[0] & ((ZU(1) << ckh->lg_curbuckets) - 1);
    if (!ckh_try_bucket_insert(ckh, bucket, key, data)) {
        return false;
    }
    
    bucket = hashes[1] & ((ZU(1) << ckh->lg_curbuckets) - 1);
    if (!ckh_try_bucket_insert(ckh, bucket, key, data)) {
        return false;
    }
    
    return ckh_evict_reloc_insert(ckh, bucket, key, data);
    // FUNCTION_ID: jemalloc_func021 - END
}

bool ckh_evict_reloc_insert(ckh_t *ckh, size_t argbucket, const void *argkey, const void *argdata) {
    // FUNCTION_ID: jemalloc_func022 - START
    const void *key = argkey;
    const void *data = argdata;
    size_t bucket = argbucket;
    
    for (unsigned iter = 0; iter < 100; iter++) {
        unsigned i = (unsigned)(ckh_prng(&ckh->prng_state) % (ZU(1) << LG_CKH_BUCKET_CELLS));
        ckhc_t *cell = &ckh->tab[(bucket << LG_CKH_BUCKET_CELLS) + i];
        
        const void *tkey = cell->key;
        const void *tdata = cell->data;
        cell->key = key;
        cell->data = data;
        key = tkey;
        data = tdata;
        
        size_t hashes[2];
        ckh->hash(key, hashes);
        size_t tbucket = hashes[1] & ((ZU(1) << ckh->lg_curbuckets) - 1);
        if (tbucket == bucket) {
            tbucket = hashes[0] & ((ZU(1) << ckh->lg_curbuckets) - 1);
        }
        
        if (tbucket == argbucket) {
            return true;
        }
        
        bucket = tbucket;
        if (!ckh_try_bucket_insert(ckh, bucket, key, data)) {
            return false;
        }
    }
    
    return true;
    // FUNCTION_ID: jemalloc_func022 - END
}

void ckh_rebuild(ckh_t *ckh, ckhc_t *aTab) {
    // FUNCTION_ID: jemalloc_func023 - START
    size_t count = ckh->count;
    size_t ncells = (ZU(1) << (ckh->lg_curbuckets + LG_CKH_BUCKET_CELLS));
    
    ckh->count = 0;
    for (size_t i = 0, nins = 0; nins < count; i++) {
        if (aTab[i].key != NULL) {
            const void *key = aTab[i].key;
            const void *data = aTab[i].data;
            ckh_try_insert(ckh, key, data);
            nins++;
        }
    }
    // FUNCTION_ID: jemalloc_func023 - END
}

size_t ckh_bucket_count(ckh_t *ckh) {
    // FUNCTION_ID: jemalloc_func024 - START
    return (ZU(1) << ckh->lg_curbuckets);
    // FUNCTION_ID: jemalloc_func024 - END
}

size_t ckh_cell_index(size_t bucket, size_t cell) {
    // FUNCTION_ID: jemalloc_func025 - START
    return (bucket << LG_CKH_BUCKET_CELLS) + cell;
    // FUNCTION_ID: jemalloc_func025 - END
}

} // namespace JemallocUtils
