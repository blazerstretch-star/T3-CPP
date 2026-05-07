#include "../../src/jemalloc_utils.h"
#include <cstring>
#include <climits>

#define ZU(x) ((size_t)(x))

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

static inline unsigned ffs_lu(unsigned long x) {
    return __builtin_ffsl(x) - 1;
}

namespace JemallocUtils {

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
