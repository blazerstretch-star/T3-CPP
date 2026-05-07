#include "lfs_functions.h"
#include <cassert>
#include <iostream>

int main() {
    lfs_off_t off1 = 0;
    assert(lfs_ctz_index(&off1) == 0 && off1 == 0);
    
    lfs_off_t off2 = 1;
    assert(lfs_ctz_index(&off2) == 0 && off2 == 1);
    
    lfs_off_t off3 = 3;
    assert(lfs_ctz_index(&off3) == 1 && off3 == 3);
    
    lfs_off_t off4 = 7;
    assert(lfs_ctz_index(&off4) == 2 && off4 == 7);
    
    std::cout << "lfs_ctz_index test passed" << std::endl;
    return 0;
}
