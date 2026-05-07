#include "lfs_functions.h"
#include <cassert>
#include <cstring>
#include <iostream>

int main() {
    lfs_config cfg = {};
    cfg.block_count = 100;
    cfg.lookahead_size = 16;
    
    uint32_t buffer[4] = {0};
    
    struct {
        lfs_block_t off;
        lfs_block_t size;
        lfs_block_t i;
        lfs_block_t begin;
        lfs_block_t ack;
        uint32_t *buffer;
    } free_state = {0, 16, 0, 0, 16, buffer};
    
    lfs_t lfs = {};
    lfs.cfg = &cfg;
    lfs.free.off = free_state.off;
    lfs.free.size = free_state.size;
    lfs.free.i = free_state.i;
    lfs.free.begin = free_state.begin;
    lfs.free.ack = free_state.ack;
    lfs.free.buffer = free_state.buffer;
    
    lfs_block_t block1;
    int result1 = lfs_alloc(&lfs, &block1);
    assert(result1 == 0);
    assert(block1 == 0);
    
    lfs_block_t block2;
    int result2 = lfs_alloc(&lfs, &block2);
    assert(result2 == 0);
    assert(block2 == 1);
    
    std::cout << "lfs_alloc test passed" << std::endl;
    return 0;
}
