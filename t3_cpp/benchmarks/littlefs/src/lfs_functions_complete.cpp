#include "lfs_functions.h"
#include <string.h>
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


// Error codes
#define LFS_ERR_OK          0
#define LFS_ERR_IO          -5
#define LFS_ERR_CORRUPT     -84
#define LFS_ERR_NOENT       -2
#define LFS_ERR_EXIST       -17
#define LFS_ERR_NOTDIR      -20
#define LFS_ERR_ISDIR       -21
#define LFS_ERR_NOTEMPTY    -39
#define LFS_ERR_BADF        -9
#define LFS_ERR_FBIG        -27
#define LFS_ERR_INVAL       -22
#define LFS_ERR_NOSPC       -28
#define LFS_ERR_NOMEM       -12
#define LFS_ERR_NOATTR      -61
#define LFS_ERR_NAMETOOLONG -36

#define LFS_CMP_EQ 0
#define LFS_CMP_LT 1
#define LFS_CMP_GT 2

// Helper functions
static inline lfs_size_t lfs_min_impl(lfs_size_t a, lfs_size_t b) {
    return (a < b) ? a : b;
}

static inline lfs_size_t lfs_max(lfs_size_t a, lfs_size_t b) {
    return (a > b) ? a : b;
}

static inline lfs_size_t lfs_aligndown_impl(lfs_size_t a, lfs_size_t alignment) {
    return a - (a % alignment);
}

static inline lfs_size_t lfs_alignup_impl(lfs_size_t a, lfs_size_t alignment) {
    return lfs_aligndown_impl(a + alignment - 1, alignment);
}

static inline uint32_t lfs_ctz_impl(uint32_t a) {
    return __builtin_ctz(a);
}

static inline uint32_t lfs_npw2(uint32_t a) {
    return 32 - __builtin_clz(a-1);
}

static inline int lfs_scmp(int32_t a, int32_t b) {
    return (a < b) ? -1 : (a > b) ? 1 : 0;
}

static inline uint32_t lfs_fromle32(uint32_t a) {
    return a;
}

static inline uint32_t lfs_tole32(uint32_t a) {
    return a;
}

// Forward declaration for lfs_bd_read
static int lfs_bd_read(lfs_t *lfs,
        const lfs_cache_t *pcache, lfs_cache_t *rcache, lfs_size_t hint,
        lfs_block_t block, lfs_off_t off,
        void *buffer, lfs_size_t size);

uint32_t lfs_crc(uint32_t crc, const void *buffer, size_t size) {
    // FUNCTION_ID: littlefs_func001 - START
    static const uint32_t rtable[16] = {
        0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
        0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
        0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
        0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c,
    };

    const uint8_t *data = static_cast<const uint8_t*>(buffer);

    for (size_t i = 0; i < size; i++) {
        crc = (crc >> 4) ^ rtable[(crc ^ (data[i] >> 0)) & 0xf];
        crc = (crc >> 4) ^ rtable[(crc ^ (data[i] >> 4)) & 0xf];
    }

    return crc;
    // FUNCTION_ID: littlefs_func001 - END
}

uint32_t lfs_ctz_index(lfs_off_t *off) {
    // FUNCTION_ID: littlefs_func002 - START
    lfs_off_t size = *off + 1;
    lfs_off_t b = size - 1;
    // Inlined lfs_npw2: return 32 - __builtin_clz(a-1)
    lfs_off_t bit = 32 - __builtin_clz(size - 1);

    if (bit > 0) {
        bit -= 1;
    }

    lfs_off_t i = 0;
    while (bit > 0 && (b & (1 << (bit-1)))) {
        i += 1;
        bit -= 1;
    }

    *off = size - (1 << bit);
    return i;
    // FUNCTION_ID: littlefs_func002 - END
}

int lfs_bd_cmp(lfs_t *lfs, const lfs_cache_t *pcache, lfs_cache_t *rcache, lfs_size_t hint, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size) {
    // FUNCTION_ID: littlefs_func003 - START
    const uint8_t *data = static_cast<const uint8_t*>(buffer);
    lfs_size_t diff = 0;

    for (lfs_off_t i = 0; i < size; i += diff) {
        uint8_t dat[8];

        // Inlined lfs_min_impl: return (a < b) ? a : b
        diff = ((size-i) < sizeof(dat)) ? (size-i) : sizeof(dat);
        int err = lfs_bd_read(lfs,
                pcache, rcache, hint-i,
                block, off+i, &dat, diff);
        if (err) {
            return err;
        }

        int res = memcmp(dat, data + i, diff);
        if (res) {
            return res < 0 ? LFS_CMP_LT : LFS_CMP_GT;
        }
    }

    return LFS_CMP_EQ;
    // FUNCTION_ID: littlefs_func003 - END
}

int lfs_alloc(lfs_t *lfs, lfs_block_t *block) {
    // FUNCTION_ID: littlefs_func004 - START
    while (true) {
        while (lfs->free.i != lfs->free.off + lfs->free.size) {
            lfs_block_t off = lfs->free.i;
            lfs->free.i += 1;
            lfs->free.ack -= 1;

            if (!(lfs->free.buffer[off / 32] & (1U << (off % 32)))) {
                *block = (lfs->free.begin + off) % lfs->cfg->block_count;
                lfs->free.buffer[off / 32] |= 1U << (off % 32);
                lfs->free.ack += 1;
                return 0;
            }
        }

        if (lfs->free.ack == 0) {
            return LFS_ERR_NOSPC;
        }

        lfs->free.off = (lfs->free.off + lfs->free.size) % lfs->cfg->lookahead_size;
        // Inlined lfs_min_impl: return (a < b) ? a : b
        lfs_size_t a = 8*lfs->cfg->lookahead_size;
        lfs_size_t b = lfs->free.ack;
        lfs->free.size = (a < b) ? a : b;
        lfs->free.i = 0;

        memset(lfs->free.buffer, 0, lfs->cfg->lookahead_size);
    }
    // FUNCTION_ID: littlefs_func004 - END
}

lfs_size_t lfs_min(lfs_size_t a, lfs_size_t b) {
    // FUNCTION_ID: littlefs_func005 - START
    return (a < b) ? a : b;
    // FUNCTION_ID: littlefs_func005 - END
}

lfs_size_t lfs_aligndown(lfs_size_t a, lfs_size_t alignment) {
    // FUNCTION_ID: littlefs_func006 - START
    return a - (a % alignment);
    // FUNCTION_ID: littlefs_func006 - END
}

lfs_size_t lfs_alignup(lfs_size_t a, lfs_size_t alignment) {
    // FUNCTION_ID: littlefs_func007 - START
    return lfs_aligndown(a + alignment - 1, alignment);
    // FUNCTION_ID: littlefs_func007 - END
}

void lfs_pair_swap(lfs_block_t pair[2]) {
    // FUNCTION_ID: littlefs_func008 - START
    lfs_block_t t = pair[0];
    pair[0] = pair[1];
    pair[1] = t;
    // FUNCTION_ID: littlefs_func008 - END
}

bool lfs_pair_isnull(const lfs_block_t pair[2]) {
    // FUNCTION_ID: littlefs_func009 - START
    return pair[0] == LFS_BLOCK_NULL || pair[1] == LFS_BLOCK_NULL;
    // FUNCTION_ID: littlefs_func009 - END
}

int lfs_pair_cmp(const lfs_block_t paira[2], const lfs_block_t pairb[2]) {
    // FUNCTION_ID: littlefs_func010 - START
    return !(paira[0] == pairb[0] || paira[1] == pairb[1] ||
             paira[0] == pairb[1] || paira[1] == pairb[0]);
    // FUNCTION_ID: littlefs_func010 - END
}

uint16_t lfs_tag_type3(lfs_tag_t tag) {
    // FUNCTION_ID: littlefs_func011 - START
    return (tag & 0x7ff00000) >> 20;
    // FUNCTION_ID: littlefs_func011 - END
}

void lfs_gstate_xor(lfs_gstate_t *a, const lfs_gstate_t *b) {
    // FUNCTION_ID: littlefs_func012 - START
    a->tag ^= b->tag;
    a->pair[0] ^= b->pair[0];
    a->pair[1] ^= b->pair[1];
    // FUNCTION_ID: littlefs_func012 - END
}

// Stub implementation of lfs_bd_read for compilation
static int lfs_bd_read(lfs_t *lfs,
        const lfs_cache_t *pcache, lfs_cache_t *rcache, lfs_size_t hint,
        lfs_block_t block, lfs_off_t off,
        void *buffer, lfs_size_t size) {
    if (!lfs->cfg || !lfs->cfg->read) {
        return LFS_ERR_IO;
    }
    return lfs->cfg->read(lfs->cfg, block, off, buffer, size);
}
