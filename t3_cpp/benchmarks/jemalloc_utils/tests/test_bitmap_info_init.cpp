#include "../src/jemalloc_utils.h"
#include <cassert>

using namespace JemallocUtils;

int main() {
    bitmap_info_t binfo;
    bitmap_info_init(&binfo, 64);
    assert(binfo.nbits == 64);
    assert(binfo.ngroups == 1);
    
    bitmap_info_init(&binfo, 128);
    assert(binfo.nbits == 128);
    assert(binfo.ngroups == 2);
    
    return 0;
}
