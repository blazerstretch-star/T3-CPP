#include "lfs_functions.h"
#include <cassert>
#include <cstring>
#include <iostream>

static int mock_read(const struct lfs_config *c, lfs_block_t block,
        lfs_off_t off, void *buffer, lfs_size_t size) {
    static const uint8_t test_data[] = "hello world test data";
    if (off + size <= sizeof(test_data)) {
        memcpy(buffer, test_data + off, size);
        return 0;
    }
    return -5;
}

int main() {
    lfs_config cfg = {};
    cfg.read = mock_read;
    
    lfs_t lfs = {};
    lfs.cfg = &cfg;
    
    const char *test1 = "hello";
    int result1 = lfs_bd_cmp(&lfs, nullptr, nullptr, 0, 0, 0, test1, 5);
    assert(result1 == LFS_CMP_EQ);
    
    const char *test2 = "world";
    int result2 = lfs_bd_cmp(&lfs, nullptr, nullptr, 0, 0, 6, test2, 5);
    assert(result2 == LFS_CMP_EQ);
    
    const char *test3 = "wrong";
    int result3 = lfs_bd_cmp(&lfs, nullptr, nullptr, 0, 0, 0, test3, 5);
    assert(result3 != LFS_CMP_EQ);
    
    std::cout << "lfs_bd_cmp test passed" << std::endl;
    return 0;
}
