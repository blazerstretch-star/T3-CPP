#include "../src/jemalloc_utils.h"
#include <cassert>

using namespace JemallocUtils;

int main() {
    size_t idx = ckh_cell_index(5, 3);
    assert(idx == 5 * (1U << LG_CKH_BUCKET_CELLS) + 3);
    
    idx = ckh_cell_index(0, 0);
    assert(idx == 0);
    
    return 0;
}
