#include "../src/jemalloc_utils.h"
#include <cassert>

using namespace JemallocUtils;

int main() {
    const char *s1 = "hello";
    const char *s2 = "hello";
    const char *s3 = "world";
    assert(ckh_string_keycomp(s1, s2));
    assert(!ckh_string_keycomp(s1, s3));
    
    return 0;
}
