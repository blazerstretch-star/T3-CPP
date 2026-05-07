#include "../src/jemalloc_utils.h"
#include <cassert>

using namespace JemallocUtils;

int main() {
    // PROPERTY 1: Hash function produces output
    int x = 10;
    size_t hash1[2];
    ckh_pointer_hash(&x, hash1);
    assert(hash1[0] != 0 || hash1[1] != 0);
    
    // PROPERTY 2: Determinism - same input produces same output
    size_t hash2[2];
    ckh_pointer_hash(&x, hash2);
    assert(hash1[0] == hash2[0] && "Hash must be deterministic");
    assert(hash1[1] == hash2[1] && "Hash must be deterministic");
    
    // PROPERTY 3: Different pointers produce different hashes (collision resistance)
    int y = 20;
    size_t hash3[2];
    ckh_pointer_hash(&y, hash3);
    assert((hash1[0] != hash3[0] || hash1[1] != hash3[1]) && 
           "Different pointers should produce different hashes");
    
    // PROPERTY 4: Produces two hash values (for cuckoo hashing)
    int z = 30;
    size_t hash4[2];
    ckh_pointer_hash(&z, hash4);
    // At least one hash should be non-zero
    assert((hash4[0] != 0 || hash4[1] != 0) && "Hash must produce non-zero values");
    
    return 0;
}
