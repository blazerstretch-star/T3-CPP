#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Error codes
#define LFS_ERR_OK          0
#define LFS_ERR_IO          -5
#define LFS_ERR_NOSPC       -28

// Comparison results
#define LFS_CMP_EQ 0
#define LFS_CMP_LT 1
#define LFS_CMP_GT 2

// LittleFS type definitions
typedef uint32_t lfs_size_t;
typedef uint32_t lfs_off_t;
typedef uint32_t lfs_block_t;

#define LFS_BLOCK_NULL ((lfs_block_t)-1)
#define LFS_BLOCK_INLINE ((lfs_block_t)-2)

struct lfs_config {
    void *context;
    int (*read)(const struct lfs_config *c, lfs_block_t block,
            lfs_off_t off, void *buffer, lfs_size_t size);
    int (*prog)(const struct lfs_config *c, lfs_block_t block,
            lfs_off_t off, const void *buffer, lfs_size_t size);
    int (*erase)(const struct lfs_config *c, lfs_block_t block);
    int (*sync)(const struct lfs_config *c);
    lfs_size_t read_size;
    lfs_size_t prog_size;
    lfs_size_t block_size;
    lfs_size_t block_count;
    int32_t block_cycles;
    lfs_size_t cache_size;
    lfs_size_t lookahead_size;
};

struct lfs_cache {
    lfs_block_t block;
    lfs_off_t off;
    lfs_size_t size;
    uint8_t *buffer;
};

typedef struct lfs_cache lfs_cache_t;

struct lfs {
    const struct lfs_config *cfg;
    lfs_block_t block_count;
    uint32_t seed;
    struct {
        lfs_block_t begin;
        lfs_block_t end;
        lfs_off_t off;
        lfs_size_t size;
        lfs_off_t i;
        lfs_size_t ack;
        uint32_t *buffer;
    } free;
    uint8_t *lookahead_buffer;
};

typedef struct lfs lfs_t;

struct lfs_ctz {
    lfs_block_t head;
    lfs_size_t size;
};

typedef uint32_t lfs_tag_t;

struct lfs_gstate {
    uint32_t tag;
    lfs_block_t pair[2];
};

typedef struct lfs_gstate lfs_gstate_t;

// Function declarations
uint32_t lfs_crc(uint32_t crc, const void *buffer, size_t size);
uint32_t lfs_ctz_index(lfs_off_t *off);
int lfs_bd_cmp(lfs_t *lfs, const lfs_cache_t *pcache, lfs_cache_t *rcache, lfs_size_t hint, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size);
int lfs_alloc(lfs_t *lfs, lfs_block_t *block);
lfs_size_t lfs_min(lfs_size_t a, lfs_size_t b);
lfs_size_t lfs_aligndown(lfs_size_t a, lfs_size_t alignment);
lfs_size_t lfs_alignup(lfs_size_t a, lfs_size_t alignment);

// Metadata pair functions
void lfs_pair_swap(lfs_block_t pair[2]);
bool lfs_pair_isnull(const lfs_block_t pair[2]);
int lfs_pair_cmp(const lfs_block_t paira[2], const lfs_block_t pairb[2]);
uint16_t lfs_tag_type3(lfs_tag_t tag);
void lfs_gstate_xor(lfs_gstate_t *a, const lfs_gstate_t *b);
