#ifndef JEMALLOC_UTILS_H
#define JEMALLOC_UTILS_H

#include <cstddef>
#include <cstdint>

// Bitmap types
typedef unsigned long bitmap_t;
#define LG_SIZEOF_BITMAP 3
#define LG_BITMAP_GROUP_NBITS (LG_SIZEOF_BITMAP + 3)
#define BITMAP_GROUP_NBITS (1U << LG_BITMAP_GROUP_NBITS)
#define BITMAP_GROUP_NBITS_MASK (BITMAP_GROUP_NBITS - 1)
#define BITMAP_BITS2GROUPS(nbits) (((nbits) + BITMAP_GROUP_NBITS_MASK) >> LG_BITMAP_GROUP_NBITS)

typedef struct {
    size_t nbits;
    size_t ngroups;
} bitmap_info_t;

// Cuckoo hash types
#define LG_CKH_BUCKET_CELLS 3

typedef void ckh_hash_t(const void *, size_t[2]);
typedef bool ckh_keycomp_t(const void *, const void *);

typedef struct {
    const void *key;
    const void *data;
} ckhc_t;

typedef struct {
    uint64_t prng_state;
    size_t count;
    unsigned lg_minbuckets;
    unsigned lg_curbuckets;
    ckh_hash_t *hash;
    ckh_keycomp_t *keycomp;
    ckhc_t *tab;
} ckh_t;

namespace JemallocUtils {

// EASY: Bitmap functions (8 functions)
void bitmap_info_init(bitmap_info_t *binfo, size_t nbits);
size_t bitmap_size(const bitmap_info_t *binfo);
bool bitmap_get(bitmap_t *bitmap, const bitmap_info_t *binfo, size_t bit);
void bitmap_set(bitmap_t *bitmap, const bitmap_info_t *binfo, size_t bit);
size_t ckh_count(ckh_t *ckh);
void ckh_pointer_hash(const void *key, size_t r_hash[2]);
bool ckh_pointer_keycomp(const void *k1, const void *k2);
bool ckh_string_keycomp(const void *k1, const void *k2);

// MEDIUM: Bitmap and hash operations (12 functions)
void bitmap_init(bitmap_t *bitmap, const bitmap_info_t *binfo, bool fill);
void bitmap_unset(bitmap_t *bitmap, const bitmap_info_t *binfo, size_t bit);
bool bitmap_full(bitmap_t *bitmap, const bitmap_info_t *binfo);
size_t bitmap_ffu(const bitmap_t *bitmap, const bitmap_info_t *binfo, size_t min_bit);
size_t bitmap_sfu(bitmap_t *bitmap, const bitmap_info_t *binfo);
void ckh_string_hash(const void *key, size_t r_hash[2]);
bool ckh_bucket_search(ckh_t *ckh, size_t bucket, const void *key);
size_t ckh_isearch(ckh_t *ckh, const void *key);
bool ckh_try_bucket_insert(ckh_t *ckh, size_t bucket, const void *key, const void *data);
bool ckh_search(ckh_t *ckh, const void *searchkey, void **key, void **data);
bool ckh_iter(ckh_t *ckh, size_t *tabind, void **key, void **data);
uint64_t ckh_prng(uint64_t *state);

// HARD: Complex cuckoo hash operations (5 functions)
bool ckh_try_insert(ckh_t *ckh, const void *key, const void *data);
bool ckh_evict_reloc_insert(ckh_t *ckh, size_t argbucket, const void *argkey, const void *argdata);
void ckh_rebuild(ckh_t *ckh, ckhc_t *aTab);
size_t ckh_bucket_count(ckh_t *ckh);
size_t ckh_cell_index(size_t bucket, size_t cell);

} // namespace JemallocUtils

#endif // JEMALLOC_UTILS_H
