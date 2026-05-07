#include "../src/jemalloc_utils.h"
#include <cassert>

using namespace JemallocUtils;

int main() {
    ckh_t ckh;
    ckh.lg_curbuckets = 4;
    size_t count = ckh_bucket_count(&ckh);
    assert(count == 16);
    
    ckh.lg_curbuckets = 5;
    count = ckh_bucket_count(&ckh);
    assert(count == 32);
    
    return 0;
}
